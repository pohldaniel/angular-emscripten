#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"
#include "AssimpModel.h"

#define DEFFERED_WGSL             "@vertex\n \
                                   fn vs_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {\n \
                                       const pos = array(                                                                \n \
                                           vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0),                           \n \
                                           vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),                             \n \
                                       );                                                                                \n \
                                                                                                                         \n \
                                       return vec4<f32>(pos[VertexIndex], 0.0, 1.0);                                     \n \
                                   }                                                                                     \n \
                                                                                                                         \n \
                                   @group(0) @binding(0) var gBufferNormal: texture_2d<f32>;                             \n \
                                   @group(0) @binding(1) var gBufferAlbedo: texture_2d<f32>;                             \n \
                                   @group(0) @binding(2) var gBufferDepth: texture_depth_2d;                             \n \
                                                                                                                         \n \
                                   struct LightData {                                                                    \n \
                                       position : vec4<f32>,                                                             \n \
                                       color : vec3<f32>,                                                                \n \
                                       radius : f32,                                                                     \n \
                                   }                                                                                     \n \
                                                                                                                         \n \
                                   struct LightsBuffer {                                                                 \n \
                                       lights: array<LightData>,                                                         \n \
                                   }                                                                                     \n \
                                                                                                                         \n \
                                   @group(1) @binding(0) var<storage, read> lightsBuffer: LightsBuffer;                  \n \
                                                                                                                         \n \
                                   struct Config {                                                                       \n \
                                       numLights : u32,                                                                  \n \
                                   }                                                                                     \n \
                                                                                                                         \n \
                                   struct Camera {                                                                       \n \
                                       viewProjectionMatrix : mat4x4<f32>,                                               \n \
                                       invViewProjectionMatrix : mat4x4<f32>,                                            \n \
                                   }                                                                                     \n \
                                                                                                                         \n \
                                   @group(1) @binding(1) var<uniform> config: Config;                                    \n \
                                   @group(1) @binding(2) var<uniform> camera: Camera;                                    \n \
                                                                                                                         \n \
                                   fn world_from_screen_coord(coord : vec2<f32>, depth_sample: f32) -> vec3<f32> {              \n \
                                       // reconstruct world-space position from the screen coordinate.                          \n \
                                       let posClip = vec4(coord.x * 2.0 - 1.0, (1.0 - coord.y) * 2.0 - 1.0, depth_sample, 1.0); \n \
                                       let posWorldW = camera.invViewProjectionMatrix * posClip;                                \n \
                                       let posWorld = posWorldW.xyz / posWorldW.www;                                            \n \
                                       return posWorld;                                                                         \n \
                                   }                                                                                            \n \
                                                                                                                                \n \
                                   @fragment                                                                                    \n \
                                   fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {                 \n \
                                       var result : vec3<f32>;                                                                  \n \
                                                                                                                                \n \
                                       let depth = textureLoad(gBufferDepth, vec2<i32>(floor(coord.xy)),0);                     \n \
                                                                                                                                \n \
                                       // Don't light the sky.                                                                  \n \
                                       if (depth >= 1.0) {                                                                      \n \
                                           discard;                                                                             \n \
                                       }                                                                                        \n \
                                                                                                                                \n \
                                       let bufferSize = textureDimensions(gBufferDepth);                                        \n \
                                       let coordUV = coord.xy / vec2<f32>(bufferSize);                                          \n \
                                       let position = world_from_screen_coord(coordUV, depth);                                  \n \
                                                                                                                                \n \
                                       let normal = textureLoad(gBufferNormal,vec2<i32>(floor(coord.xy)), 0).xyz;               \n \
                                                                                                                                \n \
                                       let albedo = textureLoad(gBufferAlbedo, vec2<i32>(floor(coord.xy)), 0).rgb;              \n \
                                                                                                                                \n \
                                       for (var i = 0u; i < config.numLights; i++) {                                                                     \n \
                                           let L = lightsBuffer.lights[i].position.xyz - position;                                                       \n \
                                           let distance = length(L);                                                                                     \n \
                                           if (distance > lightsBuffer.lights[i].radius) {                                                               \n \
                                               continue;                                                                                                 \n \
                                           }                                                                                                             \n \
                                           let lambert = max(dot(normal, normalize(L)), 0.0);                                                            \n \
                                           result += vec3<f32>(                                                                                          \n \
                                               lambert * pow(1.0 - distance / lightsBuffer.lights[i].radius, 2.0) * lightsBuffer.lights[i].color * albedo\n \
                                           );                                                                                                            \n \
                                       }                                                                                                                 \n \
                                                                                                                                                         \n \
                                       // some manual ambient                                                                                            \n \
                                       result += vec3(0.2);                                                                                              \n \
                                                                                                                                                         \n \
                                       return vec4(result, 1.0);                                                                                         \n \
                                   }"

#define DEFFERED_GBUFFER_WGSL      "struct Uniforms {                                                                                                            \n \
                                        modelMatrix : mat4x4<f32>,                                                                                               \n \
                                        normalModelMatrix : mat4x4<f32>,                                                                                         \n \
                                    }                                                                                                                            \n \
                                    struct Camera {                                                                                                              \n \
                                        viewProjectionMatrix : mat4x4<f32>,                                                                                      \n \
                                        invViewProjectionMatrix : mat4x4<f32>,                                                                                   \n \
                                    }                                                                                                                            \n \
                                                                                                                                                                 \n \
                                    @group(0) @binding(0) var<uniform> uniforms : Uniforms;                                                                      \n \
                                    @group(0) @binding(1) var<uniform> camera : Camera;                                                                          \n \
                                                                                                                                                                 \n \
                                    struct VertexOutput {                                                                                                        \n \
                                        @builtin(position) Position : vec4<f32>,                                                                                 \n \
                                        @location(0) fragNormal: vec3<f32>,    // normal in world space                                                          \n \
                                        @location(1) fragUV: vec2<f32>,                                                                                          \n \
                                    }                                                                                                                            \n \
                                                                                                                                                                 \n \
                                    @vertex                                                                                                                      \n \
                                    fn vs_main(@location(0) position : vec3<f32>, @location(1) uv : vec2<f32>, @location(2) normal : vec3<f32>) -> VertexOutput {\n \
                                        var output : VertexOutput;                                                                                               \n \
                                        let worldPosition = (uniforms.modelMatrix * vec4(position, 1.0)).xyz;                                                    \n \
                                        output.Position = camera.viewProjectionMatrix * vec4(worldPosition, 1.0);                                                \n \
                                        output.fragNormal = normalize((uniforms.normalModelMatrix * vec4(normal, 1.0)).xyz);                                     \n \
                                        output.fragUV = uv;                                                                                                      \n \
                                        return output;                                                                                                           \n \
                                    }                                                                                                                            \n \
                                                                                                                                                                 \n \
                                    struct GBufferOutput {                                                                                                       \n \
                                        @location(0) normal : vec4<f32>,                                                                                         \n \
                                        // Textures: diffuse color, specular color, smoothness, emissive etc. could go here                                      \n \
                                        @location(1) albedo : vec4<f32>,                                                                                         \n \
                                    }                                                                                                                            \n \
                                                                                                                                                                 \n \
                                    @fragment                                                                                                                    \n \
                                    fn fs_main(@location(0) fragNormal: vec3<f32>, @location(1) fragUV : vec2<f32>) -> GBufferOutput {                           \n \
                                        // faking some kind of checkerboard texture                                                                              \n \
                                        let uv = floor(30.0 * fragUV);                                                                                           \n \
                                        let c = 0.2 + 0.5 * ((uv.x + uv.y) - 2.0 * floor((uv.x + uv.y) / 2.0));                                                  \n \
                                                                                                                                                                 \n \
                                        var output : GBufferOutput;                                                                                              \n \
                                        output.normal = vec4(normalize(fragNormal), 1.0);                                                                        \n \
                                        output.albedo = vec4(c, c, c, 1.0);                                                                                      \n \
                                                                                                                                                                 \n \
                                        return output;                                                                                                           \n \
                                    }"

#define DEFFERED_COMPUTE_WGSL       "struct LightData {                                                                                                                             \n \
                                        position : vec4<f32>,                                                                                                                       \n \
                                        color : vec3<f32>,                                                                                                                          \n \
                                        radius : f32,                                                                                                                               \n \
                                    }                                                                                                                                               \n \
                                                                                                                                                                                    \n \
                                    struct LightsBuffer {                                                                                                                           \n \
                                        lights: array<LightData>,                                                                                                                   \n \
                                    }                                                                                                                                               \n \
                                    @group(0) @binding(0) var<storage, read_write> lightsBuffer: LightsBuffer;                                                                      \n \
                                                                                                                                                                                    \n \
                                    struct Config {                                                                                                                                 \n \
                                       numLights : u32,                                                                                                                             \n \
                                    }                                                                                                                                               \n \
                                    @group(0) @binding(1) var<uniform> config: Config;                                                                                              \n \
                                                                                                                                                                                    \n \
                                    struct LightExtent {                                                                                                                            \n \
                                       min : vec4<f32>,                                                                                                                             \n \
                                       max : vec4<f32>,                                                                                                                             \n \
                                    }                                                                                                                                               \n \
                                    @group(0) @binding(2) var<uniform> lightExtent: LightExtent;                                                                                    \n \
                                                                                                                                                                                    \n \
                                    @compute @workgroup_size(64, 1, 1)                                                                                                              \n \
                                    fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {                                                                        \n \
                                       var index = GlobalInvocationID.x;                                                                                                            \n \
                                       if (index >= config.numLights) {                                                                                                             \n \
                                         return;                                                                                                                                    \n \
                                       }                                                                                                                                            \n \
                                                                                                                                                                                    \n \
                                       lightsBuffer.lights[index].position.y = lightsBuffer.lights[index].position.y - 0.5 - 0.003 * (f32(index) - 64.0 * floor(f32(index) / 64.0));\n \
                                                                                                                                                                                    \n \
                                       if (lightsBuffer.lights[index].position.y < lightExtent.min.y) {                                                                             \n \
                                           lightsBuffer.lights[index].position.y = lightExtent.max.y;                                                                               \n \
                                       }                                                                                                                                            \n \
                                    }"

#define DEFFERED_DEBUG_WGSL         "@vertex                                                                               \n \
                                     fn vs_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {\n \
                                       const pos = array(                                                                  \n \
                                         vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0),                               \n \
                                         vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),                                 \n \
                                       );                                                                                  \n \
                                                                                                                           \n \
                                       return vec4<f32>(pos[VertexIndex], 0.0, 1.0);                                       \n \
                                     }                                                                                     \n \
                                                                                                                           \n \
                                     @group(0) @binding(0) var gBufferNormal: texture_2d<f32>;                             \n \
                                     @group(0) @binding(1) var gBufferAlbedo: texture_2d<f32>;                             \n \
                                     @group(0) @binding(2) var gBufferDepth: texture_depth_2d;                             \n \
                                                                                                                           \n \
                                     override canvasSizeWidth: f32;                                                        \n \
                                     override canvasSizeHeight: f32;                                                       \n \
                                                                                                                           \n \
                                     fn getDepth(z : f32, near : f32 , far : f32) -> f32{                                  \n \
                                     	let z_ndc = z * 2.0 - 1.0;                                                         \n \
                                     	return (2.0 * near * far) / (far + near + z_ndc * (near - far));                   \n \
                                     }                                                                                     \n \
                                                                                                                           \n \
                                     @fragment                                                                             \n \
                                     fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {          \n \
                                       var result : vec4<f32>;                                                             \n \
                                       let dimensions : vec2<u32> = textureDimensions(gBufferAlbedo);                      \n \
                                                                                                                           \n \
                                       let c = coord.xy / vec2<f32>(dimensions);                                           \n \
                                       if (c.x < 0.33333) {                                                                \n \
                                         let rawDepth = textureLoad(gBufferDepth, vec2<i32>(floor(coord.xy)), 0);          \n \
                                         // remap depth into something a bit more visible                                  \n \
                                         let depth = (1.0 - rawDepth * 0.993) * 50.0;                                      \n \
                                         result = vec4(depth);                                                             \n \
                                       } else if (c.x < 0.66667) {                                                         \n \
                                         result = textureLoad(                                                             \n \
                                           gBufferNormal,                                                                  \n \
                                           vec2<i32>(floor(coord.xy)),                                                     \n \
                                           0                                                                               \n \
                                         );                                                                                \n \
                                         result.x = (result.x + 1.0) * 0.5;                                                \n \
                                         result.y = (result.y + 1.0) * 0.5;                                                \n \
                                         result.z = (result.z + 1.0) * 0.5;                                                \n \
                                       } else {                                                                            \n \
                                         result = textureLoad(                                                             \n \
                                           gBufferAlbedo,                                                                  \n \
                                           vec2<i32>(floor(coord.xy)),                                                     \n \
                                           0                                                                               \n \
                                         );                                                                                \n \
                                       }                                                                                   \n \
                                       return result;                                                                      \n \
                                     }"

#define MAX_NUM_LIGHTS 1024u

class DefferedRendering : public State {

public:

	DefferedRendering(StateMachine& machine);
	~DefferedRendering();

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

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsGBuffer();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsCompute();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsDeffered();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsDefferedDebug();

	std::vector<WGPUBindGroup> OnBindGroupsGBuffer();
	WGPUBindGroup createDefferedBindGroup();
	WGPUBindGroup createLightBindGroup();
	WGPUBindGroup createComputeBindGroup();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	float randomFloat(float min, float max);

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_debug = false;
	int m_numLights = 128;

	Camera m_camera;
	TrackBall m_trackball;
	AssimpModel m_dragon;
	Shape m_quad;

	WgpBuffer m_uniformBuffer, m_cameraBuffer, m_lightBuffer, m_configBuffer, m_extentBuffer;
	WgpModel m_wgpDragon, m_wgpQuad;
	WgpTexture m_normalTexture, m_albedoTexture, m_depthTexture;
	WGPUBindGroup m_defferedBindGroup, m_lightBindGroup, m_computeBindGroup;

	std::vector<WGPURenderPassColorAttachment> renderPassColorAttachments;
	WGPURenderPassDepthStencilAttachment renderPassDepthStencilAttachment;

	static glm::vec3& RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation = glm::vec3(0.0f, 0.0f, 0.0f));

	static const glm::mat4 IDENTITY;
};