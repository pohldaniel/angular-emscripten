#pragma once
#include <list>
#include <webgpu/webgpu.h>

class WgpMesh;
class WgpBuffer;
class ObjModel;
enum RenderPipelineSlot;

class WgpModel {

public:

	WgpModel() = default;
	WgpModel(WgpModel const& rhs);
	WgpModel(WgpModel&& rhs) noexcept;

	void draw(const WGPURenderPassEncoder& renderPassEncoder) const;
    void drawRaw(const WGPURenderPassEncoder& renderPassEncoder) const;

	void create(const ObjModel& model, const WGPUTextureView& textureView, const WgpBuffer& uniformBuffer);
	void create(const ObjModel& model, const WGPURenderPipeline& renderPipeline, const WGPUBindGroup& bindGroup, const WgpBuffer& uniformBuffer);
	void setRenderPipelineSlot(RenderPipelineSlot renderPipelineSlot);
	void createBindGroup(const std::string& pipelineName);
	
private:

	std::list<WgpMesh> m_meshes;
};