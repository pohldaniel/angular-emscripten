#pragma once

#include <functional>
#include <States/StateMachine.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpData.h>

#include "Camera.h"
#include "TrackBall.h"
#include "AssimpModel.h"

class ImageBasedLighting : public State {

public:

	ImageBasedLighting(StateMachine& machine);
	~ImageBasedLighting();

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
	bool m_drawUi = false;

	Camera m_camera;
	TrackBall m_trackball;
	Uniforms m_uniforms;
	WgpBuffer m_uniformBuffer;

	AssimpModel m_helmet;
	WgpModel m_wgpHelmet;

	WgpTexture m_texture;
	void addBindgroups(const WgpModel& model);
};