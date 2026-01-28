#include <functional>
#include <States/StateMachine.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpData.h>
#include <WebGPU/WgpModel.h>
#include "Mouse.h"
#include "Camera.h"
#include "TrackBall.h"
#include "ObjModel.h"

enum Model {
	MAMMOTH,
	DRAGON
};

class Default : public State {

public:    

    Default(StateMachine& machine);
	~Default();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPURenderPassEncoder& renderPass);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;
	
private:

	WGPUBindGroupLayout OnBindGroupLayoutPTN();
	WGPUBindGroupLayout OnBindGroupLayoutWireframe();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void applyTransformation(const TrackBall& arc);

	bool m_initUi = true;
	bool m_drawUi = true;
	Model m_model = Model::DRAGON;
	Camera m_camera;
	TrackBall m_trackball;

	ObjModel m_mammoth, m_dragon;
	WgpBuffer m_uniformBuffer;
	WgpModel m_wgpDragon, m_wgpMammoth;
	Uniforms m_uniforms;
	WGPUTexture m_texture;
	WGPUTextureView m_textureView;
};