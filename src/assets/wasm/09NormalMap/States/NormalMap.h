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
#include "Shape.h"

enum TexturePack {
	SPIRAL,
	TOY_BOX
};

class NormalMap : public State {

public:

	NormalMap(StateMachine& machine);
	~NormalMap();

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

	Shape m_cube, m_sphere, m_torus, m_torusKnot, m_spiral;
	WgpBuffer m_uniformBuffer, m_normalUniformBuffer;

	Uniforms m_uniforms;
	NormalUniforms m_normalUniforms;
	WgpModel m_wgpCube, m_wgpSphere, m_wgpTorus, m_wgpTorusKnot, m_wgpSpiral;

	WgpTexture m_textureAW, m_textureNT, m_textureHT;
	WgpTexture m_textureNS, m_textureHS;
	WgpTexture m_textureAB, m_textureNB, m_textureHB;

	std::vector <WGPUBindGroup> m_bindgroups;
	TexturePack m_texturePack = TexturePack::SPIRAL;

	static glm::mat4 GetNormalMatrix(const glm::mat4& m);
	static glm::vec3 glm_vec3_transform_mat4(const glm::mat4& m, const glm::vec3& v);
};