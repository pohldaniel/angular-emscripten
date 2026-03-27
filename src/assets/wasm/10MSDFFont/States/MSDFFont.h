#pragma once

#include <functional>
#include <States/StateMachine.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpData.h>
#include <WebGPU/WgpModel.h>
#include "Camera.h"
#include "TrackBall.h"
#include "CharacterSet.h"

class MSDFFont : public State {

public:

	MSDFFont(StateMachine& machine);
	~MSDFFont();

	void fixedUpdate() override;
	void update() override;
	void render() override;

	void OnDraw(const WGPURenderPassEncoder& renderPass);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;
	
private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroup> OnBindGroups();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void applyTransformation(const TrackBall& arc);

	bool m_initUi = true;
	bool m_drawUi = true;

	Camera m_camera;
	TrackBall m_trackball;
    Uniforms m_uniforms;
	CharacterSet m_characterSet;
    WgpBuffer m_uniformBuffer;
	float m_fontSize = 0.6f;

	static glm::mat4 GetNormalMatrix(const glm::mat4& m);
};