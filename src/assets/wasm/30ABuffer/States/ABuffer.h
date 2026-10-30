#pragma once

#include <States/StateMachine.h>

#include <WebGPU/WgpData.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>

#include "Camera.h"
#include "TrackBall.h"
#include "ObjModel.h"

#define ABUFFER_OPAQUE_WGSL      "struct VertexInput {\n \
                                      @location(0) position: vec3f,\n \
	                                  @location(1) normal: vec3f\n \
                                  };\n \
\n \
                                  struct VertexOutput {\n \
	                                  @builtin(position) position: vec4f,\n \
	                                  @location(1) texcoord: vec2f,\n \
	                                  @location(2) normal: vec3f,\n \
	                                  @location(3) color: vec4f,\n \
	                                  @location(4) @interpolate(flat) instance: u32\n \
                                  };\n \
\n \
                                  struct Uniforms {\n \
                                      projection: mat4x4<f32>,\n \
	                                  view: mat4x4<f32>,\n \
	                                  env: mat4x4<f32>,\n \
                                      model: mat4x4<f32>,    \n \
	                                  normal: mat4x4<f32>,\n \
	                                  color: vec4<f32>,\n \
	                                  camPos: vec3<f32>\n \
                                  };\n \
\n \
                                  @group(0) @binding(0) var<uniform> uniforms: Uniforms;\n \
\n \
                                  @vertex\n \
                                  fn vs_main(in: VertexInput, @builtin(instance_index) instance: u32) -> VertexOutput {\n \
                                      var out: VertexOutput;\n \
\n \
                                      // distribute instances into a staggered 4x4 grid\n \
                                      const gridWidth = 125.0;\n \
                                      const cellSize = gridWidth / 4.0;\n \
                                      let row = instance / 2u;\n \
                                      let col = instance % 2u;\n \
\n \
                                      let xOffset = -gridWidth / 2.0 + cellSize / 2.0 + 2.0 * cellSize * f32(col) + f32(row % 2u != 0u) * cellSize;\n \
                                      let zOffset = -gridWidth / 2.0 + cellSize / 2.0 + 2.0 + f32(row) * cellSize;\n \
\n \
                                      let offsetPos = vec4(in.position.x + xOffset, in.position.y, in.position.z + zOffset, 1.0);\n \
\n \
                                      out.position = uniforms.projection * uniforms.view * uniforms.model * offsetPos;\n \
                                      out.color = uniforms.color;\n \
                                      out.normal = in.normal;\n \
                                      out.instance = instance;\n \
                                      return out;\n \
                                  }\n \
\n \
                                  @fragment\n \
                                  fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {\n \
                                      const colors = array<vec3<f32>,6>(\n \
                                      vec3(1.0, 0.0, 0.0),\n \
                                      vec3(0.0, 1.0, 0.0),\n \
                                      vec3(0.0, 0.0, 1.0),\n \
                                      vec3(1.0, 0.0, 1.0),\n \
                                      vec3(1.0, 1.0, 0.0),\n \
                                      vec3(0.0, 1.0, 1.0),\n \
                                      );\n \
\n \
                                      return vec4(colors[in.instance % 6u], 1.0);\n \
                                  }"


#define ABUFFER_TRANSLUCENT_WGSL   "struct VertexInput {\n \
	@location(0) position: vec3f,\n \
	@location(1) normal: vec3f\n \
};\n \
\n \
struct Uniforms {\n \
    projection: mat4x4<f32>,\n \
	view: mat4x4<f32>,\n \
	env: mat4x4<f32>,\n \
    model: mat4x4<f32>,    \n \
	normal: mat4x4<f32>,\n \
	color: vec4<f32>,\n \
	camPos: vec3<f32>\n \
};\n \
\n \
struct SliceInfo {\n \
  maxStorableFragments: u32,\n \
  targetWidth: u32,\n \
  sliceStartY: i32\n \
};\n \
\n \
struct Heads {\n \
  numFragments: atomic<u32>,\n \
  data: array<atomic<u32>>\n \
};\n \
\n \
struct LinkedListElement {\n \
  color: vec4<f32>,\n \
  depth: f32,\n \
  next: u32\n \
};\n \
\n \
struct LinkedList {\n \
  data: array<LinkedListElement>\n \
};\n \
\n \
@binding(0) @group(0) var<uniform> uniforms: Uniforms;\n \
@binding(1) @group(0) var<storage, read_write> heads: Heads;\n \
@binding(2) @group(0) var<storage, read_write> linkedList: LinkedList;\n \
@binding(3) @group(0) var opaqueDepthTexture: texture_depth_2d;\n \
@binding(4) @group(0) var<uniform> sliceInfo: SliceInfo;\n \
\n \
struct VertexOutput {\n \
  @builtin(position) position: vec4<f32>,\n \
  @location(1) texcoord: vec2f,\n \
  @location(2) normal: vec3f,\n \
  @location(3) color: vec4f,\n \
  @location(4) @interpolate(flat) instance: u32\n \
};\n \
\n \
@vertex\n \
fn vs_main(in: VertexInput, @builtin(instance_index) instance: u32) -> VertexOutput {\n \
  var out: VertexOutput;\n \
\n \
  // distribute instances into a staggered 4x4 grid\n \
  const gridWidth = 125.0;\n \
  const cellSize = gridWidth / 4.0;\n \
  let row = instance / 2u;\n \
  let col = instance % 2u;\n \
\n \
  let xOffset = -gridWidth / 2.0 + cellSize / 2.0 + 2.0 * cellSize * f32(col) + f32(row % 2u == 0u) * cellSize;\n \
  let zOffset = -gridWidth / 2.0 + cellSize / 2.0 + 2.0 + f32(row) * cellSize;\n \
\n \
  let offsetPos = vec4(in.position.x + xOffset, in.position.y, in.position.z + zOffset, 1.0);\n \
\n \
  out.position = uniforms.projection * uniforms.view * uniforms.model * offsetPos;\n \
  out.color = uniforms.color;\n \
  out.normal = in.normal;\n \
  out.instance = instance;\n \
\n \
  return out;\n \
}\n \
\n \
@fragment\n \
fn fs_main(in: VertexOutput) {\n \
  const colors = array<vec3<f32>,6>(\n \
    vec3(1.0, 0.0, 0.0),\n \
    vec3(0.0, 1.0, 0.0),\n \
    vec3(0.0, 0.0, 1.0),\n \
    vec3(1.0, 0.0, 1.0),\n \
    vec3(1.0, 1.0, 0.0),\n \
    vec3(0.0, 1.0, 1.0),\n \
  );\n \
\n \
  let fragCoords = vec2<i32>(in.position.xy);\n \
  let opaqueDepth = textureLoad(opaqueDepthTexture, fragCoords, 0);\n \
\n \
  // reject fragments behind opaque objects\n \
  if in.position.z >= opaqueDepth {\n \
    discard;\n \
  }\n \
\n \
  // The index in the heads buffer corresponding to the head data for the fragment at\n \
  // the current location.\n \
  let headsIndex = u32(fragCoords.y - sliceInfo.sliceStartY) * sliceInfo.targetWidth + u32(fragCoords.x);\n \
\n \
  // The index in the linkedList buffer at which to store the new fragment\n \
  let fragIndex = atomicAdd(&heads.numFragments, 1u);\n \
\n \
  // If we run out of space to store the fragments, we just lose them\n \
  if fragIndex < sliceInfo.maxStorableFragments {\n \
    let lastHead = atomicExchange(&heads.data[headsIndex], fragIndex);\n \
    linkedList.data[fragIndex].depth = in.position.z;\n \
    linkedList.data[fragIndex].next = lastHead;\n \
    linkedList.data[fragIndex].color = vec4(colors[(in.instance + 3u) % 6u], 0.3);\n \
  }\n \
}"

#define ABUFFER_COMPOSITE_WGSL  "struct SliceInfo {\n \
  maxStorableFragments: u32,\n \
  targetWidth: u32,\n \
  sliceStartY: i32\n \
};\n \
\n \
struct Heads {\n \
  numFragments: u32,\n \
  data: array<u32>\n \
};\n \
\n \
struct LinkedListElement {\n \
  color: vec4<f32>,\n \
  depth: f32,\n \
  next: u32\n \
};\n \
\n \
struct LinkedList {\n \
  data: array<LinkedListElement>\n \
};\n \
\n \
@binding(0) @group(0) var<storage, read_write> heads: Heads;\n \
@binding(1) @group(0) var<storage, read_write> linkedList: LinkedList;\n \
@binding(2) @group(0) var<uniform> sliceInfo: SliceInfo;\n \
\n \
@vertex\n \
fn vs_main(@builtin(vertex_index) vertIndex: u32) -> @builtin(position) vec4<f32> {\n \
  const position = array<vec2<f32>, 6>(\n \
    vec2(-1.0, -1.0),\n \
    vec2(1.0, -1.0),\n \
    vec2(1.0, 1.0),\n \
    vec2(-1.0, -1.0),\n \
    vec2(1.0, 1.0),\n \
    vec2(-1.0, 1.0),\n \
  );\n \
\n \
  return vec4(position[vertIndex], 0.0, 1.0);\n \
}\n \
\n \
@fragment\n \
fn fs_main(@builtin(position) position: vec4<f32>) -> @location(0) vec4<f32> {\n \
  let fragCoords = vec2<i32>(position.xy);\n \
  let headsIndex = u32(fragCoords.y - sliceInfo.sliceStartY) * sliceInfo.targetWidth + u32(fragCoords.x);\n \
\n \
  // The maximum layers we can process for any pixel\n \
  const maxLayers = 24u;\n \
\n \
  var layers: array<LinkedListElement, maxLayers>;\n \
\n \
  var numLayers = 0u;\n \
  var elementIndex = heads.data[headsIndex];\n \
\n \
  // copy the list elements into an array up to the maximum amount of layers\n \
  while elementIndex != 0xFFFFFFFFu && numLayers < maxLayers {\n \
    layers[numLayers] = linkedList.data[elementIndex];\n \
    numLayers++;\n \
    elementIndex = linkedList.data[elementIndex].next;\n \
  }\n \
\n \
  if numLayers == 0u {\n \
    discard;\n \
  }\n \
\n \
  // sort the fragments by depth\n \
  for (var i = 1u; i < numLayers; i++) {\n \
    let toInsert = layers[i];\n \
    var j = i;\n \
\n \
    while j > 0u && toInsert.depth > layers[j - 1u].depth {\n \
      layers[j] = layers[j - 1u];\n \
      j--;\n \
    }\n \
\n \
    layers[j] = toInsert;\n \
  }\n \
\n \
  // pre-multiply alpha for the first layer\n \
  var color = vec4(layers[0].color.a * layers[0].color.rgb, layers[0].color.a);\n \
\n \
  // blend the remaining layers\n \
  for (var i = 1u; i < numLayers; i++) {\n \
    let mixed = mix(color.rgb, layers[i].color.rgb, layers[i].color.aaa);\n \
    color = vec4(mixed, color.a);\n \
  }\n \
\n \
  return color;\n \
}"

class ABuffer : public State {
	struct SliceInfo {
		uint32_t maxStorableFragments;
		uint32_t targetWidth;
		int32_t  sliceStartY;
		uint32_t padding;
	};

	enum Setting {
		MULTIPASS,
		CLAMP
	};
public:

	ABuffer(StateMachine& machine);
	~ABuffer();

	void fixedUpdate() override;
	void update() override;
	void render() override;
    void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);
	void OnPostDraw();

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(const Event::MouseWheelEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsTranslucent();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsComposite();

	std::vector<WGPUBindGroup> OnBindGroups();
	WGPUBindGroup createBindGroupTranslucent();
	WGPUBindGroup createBindGroupComposite();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void createBuffer();
	bool m_initUi = true;
	bool m_drawUi = true;

	Camera m_camera;
	Uniforms m_uniforms;
	TrackBall m_trackball;
	ObjModel m_teapot;

	WgpBuffer m_uniformBuffer, m_sliceInfoBuffer, m_linkedListBuffer, m_headsInitBuffer, m_headsBuffer;
	WgpModel m_wgpTeapot;
	WGPUBindGroup m_bindGroupTranslucent, m_bindGroupComposite;

	const uint32_t AverageLayers = 4u;

	uint32_t m_numSlices;
	uint32_t m_sliceHeight;
	Setting m_setting = MULTIPASS;

	static glm::vec3& RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation = glm::vec3(0.0f, 0.0f, 0.0f));
};