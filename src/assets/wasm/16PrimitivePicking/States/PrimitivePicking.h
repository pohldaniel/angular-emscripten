#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>

#include "Camera.h"
#include "TrackBall.h"
#include "ObjModel.h"

#define PICK_WGSL  "struct VertexInput {\n \
	                    @location(0) position: vec3f,\n \
	                    @location(1) normal: vec3f,\n \
	                    @location(2) primitiveIndex: u32,\n \
	                    @builtin(vertex_index) vertexIndex : u32\n \
                    };\n \
\n \
                    struct Uniforms {\n \
                        projection: mat4x4<f32>,\n \
	                    view: mat4x4<f32>,\n \
	                    env: mat4x4<f32>,\n \
                        model: mat4x4<f32>,\n \
	                    normal: mat4x4<f32>,\n \
	                    color: vec4<f32>,\n \
	                    camPos: vec3<f32>,\n \
	                    lightVP: mat4x4<f32>,\n \
	                    shadow: mat4x4<f32>,\n \
	                    lightPos: vec3<f32>\n \
                    };\n \
\n \
                    struct Frame {\n \
                        viewProjectionMatrix : mat4x4f,\n \
                        invViewProjectionMatrix : mat4x4f,\n \
                        pickCoord : vec2u,\n \
                        pickedPrimitive : u32,\n \
                    }\n \
\n \
                    @group(0) @binding(0) var<uniform> uniforms : Uniforms;\n \
                    @group(0) @binding(1) var<uniform> frame : Frame;\n \
\n \
                    struct VertexOutput {\n \
                        @builtin(position) position : vec4f,\n \
                        @location(0) normal : vec3f,\n \
                        @location(1) @interpolate(flat) vertexIndex : u32,\n \
                        @location(2) @interpolate(flat) primitiveIndex : u32,\n \
                    }\n \
\n \
                    @vertex\n \
                    fn vs_main(in: VertexInput) -> VertexOutput {\n \
                        var out : VertexOutput;\n \
                        let worldPosition = (uniforms.model * vec4(in.position, 1.0)).xyz;\n \
                        out.position = frame.viewProjectionMatrix * vec4(worldPosition, 1.0);\n \
                        out.normal = normalize((uniforms.normal * vec4(in.normal, 1.0)).xyz);\n \
                        out.vertexIndex = in.vertexIndex;\n \
                        out.primitiveIndex = in.primitiveIndex;\n \
                        return out;\n \
                    }\n \
\n \
                    struct PassOutput {\n \
                        @location(0) primitive : u32,\n \
                        @location(1) color : vec4f,\n \
                    }\n \
\n \
                    @fragment\n \
                    fn fs_main(in: VertexOutput) -> PassOutput {\n \
                        let primIndex = in.primitiveIndex;\n \
\n \
                        let lightDirection = normalize(vec3f(4, 10, 6));\n \
                        let light        = dot(normalize(in.normal), lightDirection) * 0.5 + 0.5;\n \
                        let surfaceColor = vec4f(0.8, 0.8, 0.8, 1.0);\n \
\n \
                        var output : PassOutput;\n \
\n \
                        if (primIndex + 1 == frame.pickedPrimitive) {\n \
                            output.color = vec4f(1.0, 1.0, 0.0, 1.0);\n \
                        }\n \
                        else {\n \
                            output.color = vec4f(surfaceColor.xyz * light, surfaceColor.a);\n \
                        }\n \
                        output.primitive = primIndex + 1;\n \
                        return output;\n \
                    }"

#define PICK_COMPUTE_WGSL  "struct Frame {                                                                      \n \
                                viewProjectionMatrix : mat4x4f,                                                 \n \
                                invViewProjectionMatrix : mat4x4f,                                              \n \
                                pickCoord : vec2f,                                                              \n \
                                pickedPrimitive : u32,                                                          \n \
                            }                                                                                   \n \
                            @group(0) @binding(0) var<storage, read_write> frame : Frame;                       \n \
                            @group(0) @binding(1) var primitiveTex : texture_2d<u32>;                           \n \
                                                                                                                \n \
                            @compute @workgroup_size(1)                                                         \n \
                            fn cs_main() {                                                                      \n \
                                let texel = vec2u(frame.pickCoord);                                             \n \
                                frame.pickedPrimitive = textureLoad(primitiveTex, texel, 0).x;                  \n \
                            }"

#define PICK_DEBUG_WGSL "@vertex                                                                                \n \
                         fn vs_main(@builtin(vertex_index) VertexIndex : u32) ->@builtin(position) vec4f {      \n \
                             const pos                                                                          \n \
                             = array(vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0),                        \n \
                               vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0), );                             \n \
                                                                                                                \n \
                             return vec4f(pos[VertexIndex], 0.0, 1.0);                                          \n \
                         }                                                                                      \n \
                                                                                                                \n \
                         @group(0) @binding(0) var primitiveTex: texture_2d<u32>;                               \n \
                                                                                                                \n \
                         @fragment                                                                              \n \
                         fn fs_main(@builtin(position) coord : vec4f) -> @location(0) vec4f {                   \n \
                             let primitiveIndex = textureLoad(primitiveTex, vec2i(floor(coord.xy)), 0).x;       \n \
                             var result : vec4f;                                                                \n \
                             result.r = f32(primitiveIndex % 8) / 8;                                            \n \
                             result.g = f32((primitiveIndex / 8) % 8) / 8;                                      \n \
                             result.b = f32((primitiveIndex / 64) % 8) / 8;                                     \n \
                             result.a = 1.0;                                                                    \n \
                             return result;                                                                     \n \
                         }"

class PrimitivePicking : public State {

	struct Vertex {
		std::array<float, 3> position;
		std::array<float, 3> normal;
		unsigned int primitiveId;
	};

public:

	PrimitivePicking(StateMachine& machine);
	~PrimitivePicking();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);

	
	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsPick();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsCompute();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsDebug();

	std::vector<WGPUBindGroup> OnBindGroupsPick();
	WGPUBindGroup createComputeBindGroup();
	WGPUBindGroup createDebugBindGroup();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_debug = false;

	Camera m_camera;
	ObjModel m_teapot;
	Uniforms m_uniforms;
	TrackBall m_trackball;

	WgpModel m_wgpTeapot;
	WgpBuffer m_uniformBuffer, m_computeBuffer, m_stagingBuffer, m_vertexBuffer, m_indexBuffer;

	WGPUBindGroup m_computeBindGroup, m_debugBindGroup;
	WgpTexture m_indexTexture;

	std::vector<WGPURenderPassColorAttachment> renderPassColorAttachments;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

	static glm::vec3& RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation = glm::vec3(0.0f, 0.0f, 0.0f));
};