#pragma once

#include <filesystem>
#include <webgpu/webgpu.h>
#include <States/StateMachine.h>

class Shape : public State {

public:

	Shape(StateMachine& machine);
	~Shape();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	
	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;

private:

	//bool loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData);
	
};