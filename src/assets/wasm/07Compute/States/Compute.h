#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <webgpu/webgpu.h>

#include <States/StateMachine.h>

#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpData.h>


enum class FilterType {
  Sum,
  Maximum,
  Minimum,
};

struct Parameters {
  FilterType filterType = FilterType::Sum;
  glm::mat3x4 kernel = glm::mat3x4(1.0);
  bool normalize = true;
};

class Compute : public State {

public:

	Compute(StateMachine& machine);
	~Compute();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPURenderPassEncoder& renderPass);
    void compute();

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;

private:

	WGPUBindGroupLayout OnBindGroupLayout();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder, bool force = false);
	WGPUBindGroup createBindGroup(const WGPUTextureView& inputTextureView, const WGPUTextureView& outputTextureView, const WGPUBuffer& uniformBuffer);

	bool m_shouldCompute = true;
	bool m_force = true;
	
	Uniforms_Compute m_uniforms;
	Parameters m_parameters;
	float m_scale = 0.5f;
	WgpBuffer m_uniformBuffer;
	WgpTexture m_inputTexture, m_outputTexture;
	WGPUBindGroup m_bindGroup;
	WGPUTextureView m_inputTextureView;
	WGPUTextureView m_outputTextureView;
};