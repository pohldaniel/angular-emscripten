#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>

#include <WebGPU/WgpContext.h>

#include "Compute.h"
#include "Application.h"
#include "Mouse.h"

Compute::Compute(StateMachine& machine) : State(machine, States::COMPUTE) {

	m_uniformBuffer.createBuffer(sizeof(Uniforms_Compute), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	wgpContext.addSampler(wgpCreateSampler());

	wgpContext.addSahderModule("COMPUTE", "res/shader/compute.wgsl");
	wgpContext.createComputePipeline("COMPUTE", "CP_COMPUTE", std::bind(&Compute::OnBindGroupLayout, this));

	m_inputTexture.loadFromFile("res/textures/input.jpg", true);
	m_outputTexture.createEmpty(m_inputTexture.getWidth(), m_inputTexture.getHeight(), WGPUTextureUsage_TextureBinding | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc, WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm);
	m_inputTextureView = wgpCreateTextureView(m_inputTexture.getFormat(), WGPUTextureAspect::WGPUTextureAspect_All, m_inputTexture.getTexture());
	m_outputTextureView = wgpCreateTextureView(m_outputTexture.getFormat(), WGPUTextureAspect::WGPUTextureAspect_All, m_outputTexture.getTexture());
	m_bindGroup = createBindGroup(m_inputTextureView, m_outputTextureView, m_uniformBuffer.getBuffer());

    m_uniforms.kernel[0][0] = 1.0f / 3.0f; m_uniforms.kernel[1][1] = 1.0f / 3.0f; m_uniforms.kernel[2][2] = 1.0f / 3.0f;

	wgpContext.OnDraw = std::bind(&Compute::OnDraw, this, std::placeholders::_1);
}

Compute::~Compute() {
	m_uniformBuffer.markForDelete();
	m_inputTexture.markForDelete();
	m_outputTexture.markForDelete();

	wgpuBindGroupRelease(m_bindGroup);
	m_bindGroup = NULL;

	wgpuTextureViewRelease(m_inputTextureView);
	m_inputTextureView = NULL;

	wgpuTextureViewRelease(m_outputTextureView);
	m_outputTextureView = NULL;
}

void Compute::fixedUpdate() {

}

void Compute::update() {
	
}

void Compute::render() {
	wgpDraw();
	compute();
}

void Compute::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {
	renderUi(renderPassEncoder, m_force);
}

void Compute::compute() {
	if (m_shouldCompute) {
		wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms_Compute));

		WGPUCommandEncoderDescriptor commandEncoderDesc = {};
		commandEncoderDesc.label = "command_encoder";
		WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpContext.device, &commandEncoderDesc);


		WGPUComputePassDescriptor computePassDesc = {};
		computePassDesc.timestampWrites = NULL;
		WGPUComputePassEncoder computePass = wgpuCommandEncoderBeginComputePass(encoder, &computePassDesc);
		wgpuComputePassEncoderSetPipeline(computePass, wgpContext.computePipelines.at("CP_COMPUTE"));

		for (uint32_t i = 0; i < 1; ++i) {
			wgpuComputePassEncoderSetBindGroup(computePass, 0, m_bindGroup, 0, nullptr);
			uint32_t invocationCountX = m_inputTexture.getWidth();
			uint32_t invocationCountY = m_inputTexture.getHeight();
			uint32_t workgroupSizePerDim = 8;
			uint32_t workgroupCountX = (invocationCountX + workgroupSizePerDim - 1) / workgroupSizePerDim;
			uint32_t workgroupCountY = (invocationCountY + workgroupSizePerDim - 1) / workgroupSizePerDim;
			wgpuComputePassEncoderDispatchWorkgroups(computePass, workgroupCountX, workgroupCountY, 1);
		}

		wgpuComputePassEncoderEnd(computePass);
		WGPUCommandBufferDescriptor commandBufferDescriptor = {};
		commandBufferDescriptor.label = "command_buffer";
		WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, &commandBufferDescriptor);

		wgpuQueueSubmit(wgpContext.queue, 1, &commands);
		m_shouldCompute = false;
		m_force = false;
	}
}

void Compute::OnMouseMotion(const Event::MouseMoveEvent& event) {
	
}

void Compute::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
    if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}
}

void Compute::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
    if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 
}

void Compute::OnKeyDown(const Event::KeyboardEvent& event) {

}

void Compute::OnKeyUp(const Event::KeyboardEvent& event) {

}

void Compute::resize(int deltaW, int deltaH) {

}

void Compute::renderUi(const WGPURenderPassEncoder& renderPassEncoder, bool force) {
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Display images
	{
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		float offset = 0.0f;
		float width = 0.0f;

		// Input image
		width = m_inputTexture.getWidth() * m_scale;
		drawList->AddImage((ImTextureID)m_inputTextureView, { offset, 0 }, {
			offset + width,
			m_inputTexture.getHeight() * m_scale
		});
		offset += width;

		// Output image
		width = m_outputTexture.getWidth() * m_scale;
		drawList->AddImage((ImTextureID)m_outputTextureView, { offset, 0 }, {
			offset + width,
			m_outputTexture.getHeight() * m_scale
		});
		offset += width;
	}

	bool changed = false;
	ImGui::Begin("Parameters");
	float minimum = m_parameters.normalize  ? 0.0f : -2.0f;
	float maximum = m_parameters.normalize ? 4.0f : 2.0f;
	changed = ImGui::Combo("Filter Type", (int*)&m_parameters.filterType, "Sum\0Maximum\0Minimum\0") || changed;
	changed = ImGui::SliderFloat3("Kernel X", glm::value_ptr(m_parameters.kernel[0]), minimum, maximum) || changed;
	changed = ImGui::SliderFloat3("Kernel Y", glm::value_ptr(m_parameters.kernel[1]), minimum, maximum) || changed;
	changed = ImGui::SliderFloat3("Kernel Z", glm::value_ptr(m_parameters.kernel[2]), minimum, maximum) || changed;
	changed = ImGui::Checkbox("Normalize", &m_parameters.normalize) || changed;
	ImGui::End();

	if (changed) {
		float sum = glm::dot(glm::vec4(1.0, 1.0, 1.0, 0.0), m_parameters.kernel * glm::vec3(1.0));
		m_uniforms.kernel = m_parameters.normalize && std::abs(sum) > 1e-6
			? m_parameters.kernel / sum
			: m_parameters.kernel;
		m_uniforms.filterType = (uint32_t)m_parameters.filterType;
	}
	m_shouldCompute = changed || force;

	ImGui::Begin("Settings");
	ImGui::SliderFloat("Scale", &m_scale, 0.0f, 2.0f);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

WGPUBindGroupLayout Compute::OnBindGroupLayout() {
	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);

	WGPUBindGroupLayoutEntry& inputImageLayout = bindingLayoutEntries[0];
	inputImageLayout.binding = 0;
	inputImageLayout.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	inputImageLayout.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;
	inputImageLayout.visibility = WGPUShaderStage_Compute;

	WGPUBindGroupLayoutEntry& outputImageLayout = bindingLayoutEntries[1];
	outputImageLayout.binding = 1;
	outputImageLayout.storageTexture.access = WGPUStorageTextureAccess::WGPUStorageTextureAccess_WriteOnly;
	outputImageLayout.storageTexture.format = WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm;
	outputImageLayout.storageTexture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;
	outputImageLayout.visibility = WGPUShaderStage_Compute;

	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries[2];
	uniformLayout.binding = 2;
	uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms_Compute);
	uniformLayout.visibility = WGPUShaderStage_Compute;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();
	return wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
}

WGPUBindGroup Compute::createBindGroup(const WGPUTextureView& inputTextureView, const WGPUTextureView& outputTextureView, const WGPUBuffer& uniformBuffer) {
	std::vector<WGPUBindGroupEntry> entries(3);

	entries[0].binding = 0;
	entries[0].textureView = inputTextureView;

	entries[1].binding = 1;
	entries[1].textureView = outputTextureView;

	entries[2].binding = 2;
	entries[2].buffer = uniformBuffer;
	entries[2].offset = 0;
	entries[2].size = sizeof(Uniforms_Compute);

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuComputePipelineGetBindGroupLayout(wgpContext.computePipelines.at("CP_COMPUTE"), 0);
	bindGroupDesc.entryCount = (uint32_t)entries.size();
	bindGroupDesc.entries = (WGPUBindGroupEntry*)entries.data();
	return wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);
}