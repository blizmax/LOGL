#include <DeferredShadingPass.h>

DeferredShadingPass::DeferredShadingPass(float width, float height, float workGroupsX, Scene* scene, GLuint _irradianceMap) :
	mWidth(width),
	mHeight(height),
	mWorkGroupsX(workGroupsX),
	shaderLightingPass("shader/deferred_shading.vert", "shader/deferred_shading.frag"),
	irradianceMap(_irradianceMap)
{
	this->scene = scene;
	this->linearColorBuffer = scene->getTextureMap("linearColorBuffer");
	this->gSpecular = scene->getTextureMap("gSpecular");
	this->gNormal = scene->getTextureMap("gNormal");
	this->gAlbedoSpec = scene->getTextureMap("gAlbedoSpec");
	this->rboDepth = scene->getTextureMap("rboDepth");
	this->depthMap = scene->getTextureMap("depthMap");
	this->prevColorFrame1 = scene->getTextureMap("prevColorFrame1");
	this->blueNoiseTex = scene->getTextureMap("blueNoiseTex");
	this->BRDFLut = scene->getTextureMap("BRDFLut");
	this->ssaoColor = scene->getTextureMap("ssaoColor");
}

DeferredShadingPass::~DeferredShadingPass(){

}

void DeferredShadingPass::init(){


	shaderLightingPass.Use();
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gSpecular"), 0);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gAlbedoSpec"), 2);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "shadowMap"), 3);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "sceneDepth"), 4);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "prevFrame1"), 5);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "blueNoise"), 6);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "BRDFLut"), 7);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "irradianceMap"), 8);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "occlusion"), 9);

	linearFBO.Bind();
	linearFBO.AttachTexture(0, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearColorBuffer->textureID);
	linearFBO.AttachTexture(0, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rboDepth->textureID);
}

void DeferredShadingPass::update(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::mat4& previousProjection,
	const glm::mat4& previousView,
	const glm::mat4& lightSpaceMatrix,
	const glm::vec3& viewPos,
	const glm::vec3& lightPos,
	const float& tempRoughness,
	const float& currentFrameIndex,
	const float& resolve,
	const float& binaryIteration,
	const float& pixelStride,
	const int& depthLevel,
	const float& initStep,
	const float& sampleBias,
	const bool& flagShadowMap,
	const ShadowMapping& csmPass
	)
{
	this->csmInt = csmInt;
	linearFBO.Bind();
	shaderLightingPass.Use();
	shaderLightingPass.SetUniform("flagShadowMap", flagShadowMap);
	shaderLightingPass.SetUniform("_ProjectionMatrix", projection);
	shaderLightingPass.SetUniform("ViewMatrix", view);
	shaderLightingPass.SetUniform("preProjectionMatrix", previousProjection);
	shaderLightingPass.SetUniform("preViewMatrix", previousView);
	shaderLightingPass.SetUniform("inverseViewMatrix", glm::inverse(view));
	shaderLightingPass.SetUniform("inverseProjectionMatrix", glm::inverse(projection));
	shaderLightingPass.SetUniform("extRand1", dist(mt));
	shaderLightingPass.SetUniform("tempRoughness", tempRoughness);
	shaderLightingPass.SetUniform("frameIndex", currentFrameIndex);
	shaderLightingPass.SetUniform("resolve", resolve);
	shaderLightingPass.SetUniform("binaryIteration", binaryIteration);
	shaderLightingPass.SetUniform("inputStride", pixelStride);
	shaderLightingPass.SetUniform("screenWidth", (float)mWidth);
	shaderLightingPass.SetUniform("screenHeight", (float)mHeight);
	shaderLightingPass.SetUniform("numberOfTilesX", mWorkGroupsX);
	shaderLightingPass.SetUniform("directionLightDir", glm::vec3(0, 1, 1));
	shaderLightingPass.SetUniform("directionLightColor", glm::vec4(0,0,0,1));
	glUniformMatrix4fv(glGetUniformLocation(shaderLightingPass.Program, "textureMatrixList"), csmPass.m_num_splits, GL_FALSE, glm::value_ptr(csmPass.m_texture_matrices[0]));
	glUniform4fv(glGetUniformLocation(shaderLightingPass.Program, "farbounds"), 1, &csmPass.m_far_bounds[0]);
	//shaderLightingPass.SetUniform("directionLightColor", glm::vec4(245.0/256.0, 208.0/256.0, 129.0/256.0, 1));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gSpecular->textureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal->textureID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec->textureID);
	glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, depthMap->textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, csmPass.m_texture_array);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, rboDepth->textureID);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, prevColorFrame1->textureID);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, blueNoiseTex->textureID);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, BRDFLut->textureID);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	shaderLightingPass.BindTexture(9, ssaoColor->textureID, "ssaoColor");
	//glActiveTexture(GL_TEXTURE9);
	//glBindTexture(GL_TEXTURE_2D, ssaoColor->textureID);


	glUniformMatrix4fv(glGetUniformLocation(shaderLightingPass.Program, "LightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "viewPos"), 1, &viewPos[0]);
}

void DeferredShadingPass::execute(){
	linearFBO.Bind();
	RenderBufferQuad();
}

