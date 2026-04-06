struct VertexInput {
	@location(0) pos: vec3f,
	@location(1) tex: vec2f,
	@location(2) color: vec4f,
	@location(3) layer: u32
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) uv: vec2f,
	@location(1) color: vec4f,
	@location(2) @interpolate(flat) layer: u32
};

struct Uniforms {
    projectionMatrix: mat4x4f,
    viewMatrix: mat4x4f,
    modelMatrix: mat4x4f,
	normalMatrix: mat4x4f,
    color: vec4f,
	camPosition: vec3f,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var Sampler: sampler;

@group(1) @binding(0) var texture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.position = uniforms.projectionMatrix * vec4<f32>(in.pos, 0.0, 1.0);
	out.uv = in.tex;
	out.color = in.color;
	out.layer = in.layer;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

	let sample = textureSample(texture, Sampler, in.uv);
	return sample;
}