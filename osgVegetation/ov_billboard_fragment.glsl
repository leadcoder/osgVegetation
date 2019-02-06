#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
#pragma import_defines (BLT_ROTATED_QUAD)
uniform sampler2DArray ov_billboard_texture;
varying vec2 ov_geometry_texcoord;
varying vec4 ov_geometry_color;
flat varying int ov_geometry_tex_index;
varying vec3 ov_geometry_normal;
varying float ov_depth;
flat varying float ov_fade;

//forward declarations
vec3 ov_directionalLight(vec3 normal);
vec3 ov_directionalLightShadow(vec3 normal);
vec3 ov_applyFog(vec3 color, float depth);

//get billboard normal
vec3 ov_get_billboard_normal()
{
#ifdef BLT_ROTATED_QUAD
	//Create some fake normals
	float ny = 0;
	float nx = (ov_geometry_texcoord.x) * 2.0 - 1.0;
	float nz = sqrt(1.0 - (nx*nx));
	return normalize(vec3(4 * nx, ny, nz));// +ov_geometry_normal);
#else
	return normalize(ov_geometry_normal);
#endif
}

void main(void) 
{
	vec4 out_color = ov_geometry_color * texture2DArray(ov_billboard_texture, vec3(ov_geometry_texcoord, ov_geometry_tex_index));
	
	vec3 normal = ov_get_billboard_normal();

#ifndef BLT_ROTATED_QUAD //self shadows don't work well for rotated quads
	out_color.xyz *= ov_directionalLightShadow(normal);
#else
	out_color.xyz *= ov_directionalLight(normal);
#endif

	out_color.xyz = ov_applyFog(out_color.xyz, ov_depth);
	out_color.a *= ov_fade;
	gl_FragColor = out_color;
}