#version 400 compatibility
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
#pragma import_defines (BLT_ROTATED_QUAD)
uniform sampler2DArray ov_billboard_texture;
uniform float ov_billboard_color_impact;

in ov_BillboardVertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
  vec4 Color;
  float Depth;
  flat int TextureArrayIndex;
  flat float Fade;
} ov_bb_in;

out vec4 fragColor;

//forward declarations
vec3 ov_directionalLight(vec3 normal,vec3 diffuse);
vec3 ov_directionalLightShadow(vec3 normal,vec3 diffuse);
vec3 ov_applyFog(vec3 color, float depth);

//get billboard normal
vec3 ov_get_billboard_normal()
{
#ifdef BLT_ROTATED_QUAD
	//Create some fake normals
	float ny = 0;
	float nx = (ov_bb_in.TexCoord0.x) * 2.0 - 1.0;
	float nz = sqrt(1.0 - (nx*nx));
	return normalize(vec3(4 * nx, ny, nz));
#else
	return normalize(ov_bb_in.Normal);
#endif
}

void main(void) 
{
	float intensity = ov_bb_in.Color.w;
	vec4 tex_color = texture2DArray(ov_billboard_texture, vec3(ov_bb_in.TexCoord0, ov_bb_in.TextureArrayIndex));

	//modulate colors
	vec3 mod_color = ov_bb_in.Color.xyz * 2.6 * tex_color.y;
	vec3 mixed_color = mix(tex_color.xyz, mod_color, ov_billboard_color_impact) * intensity; 
	vec4 out_color = vec4(mixed_color, tex_color.a);

	vec3 normal = ov_get_billboard_normal();

#ifndef BLT_ROTATED_QUAD //self shadows don't work well for rotated quads
	out_color.xyz *= ov_directionalLightShadow(normal,vec3(1.0));
#else
	out_color.xyz *= ov_directionalLight(normal, vec3(1.0));
#endif

	out_color.xyz = ov_applyFog(out_color.xyz, ov_bb_in.Depth);
	out_color.a *= ov_bb_in.Fade;
	fragColor = out_color;
}