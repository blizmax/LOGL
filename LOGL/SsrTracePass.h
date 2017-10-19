#pragma once

#include <RenderPass.h>
#include <random>

class SsrTracePass : public RenderPass{
public:
	SsrTracePass(float width, float height, Scene *scene);
	~SsrTracePass();
	void init();
	void update(
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::mat4& previousProjection,
		const glm::mat4& previousView,
		const glm::vec3& viewPos,
		const float& tempRoughness,
		const float& currentFrameIndex,
		const float& resolve,
		const float& binaryIteration,
		const float& pixelStride,
		const int& depthLevel,
		const float& initStep,
		const float& sampleBias,
		const bool& flagHiZ
		);
	void execute();
private:
	int mWidth;
	int mHeight;
	Shader ssrTrace;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 previousView;
	glm::mat4 previousProjection;

	Framebuffer SSRHitpointFBO;
	TextureMap *gSpecular;
	TextureMap *gNormal;
	TextureMap *gAlbedoSpec;
	TextureMap *depthMap;
	TextureMap *rboDepth;
	TextureMap *prevColorFrame1;
	TextureMap *blueNoiseTex;
	TextureMap *BRDFLut;
	TextureMap *SSRHitPoint;
	Scene *scene;
	glm::vec3 viewPos;

	std::random_device rd;
	std::mt19937 mt;
	std::uniform_real_distribution<float> dist;
};