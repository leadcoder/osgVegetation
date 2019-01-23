#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
#pragma import_defines (SM_LISPSM,SM_VDSM1,SM_VDSM2,FM_LINEAR,FM_EXP,FM_EXP2, BLT_ROTATED_QUAD)
uniform sampler2DArray ov_billboard_texture;
varying vec2 ov_geometry_texcoord;
varying vec4 ov_geometry_color;
flat varying int ov_geometry_tex_index;
varying vec3 ov_geometry_normal;
varying float ov_depth;

#ifdef SM_LISPSM
uniform sampler2DShadow shadowTexture;
uniform int shadowTextureUnit;
#endif

#ifdef SM_VDSM1
uniform sampler2DShadow shadowTexture0;
uniform int shadowTextureUnit0;
#endif

#ifdef SM_VDSM2
uniform sampler2DShadow shadowTexture0;
uniform int shadowTextureUnit0;
uniform sampler2DShadow shadowTexture1;
uniform int shadowTextureUnit1;
#endif		

float ov_shadow(float value)
{
	float shadow = value;
#ifdef SM_LISPSM
	shadow *= shadow2DProj(shadowTexture, gl_TexCoord[shadowTextureUnit]).r;
#endif

#ifdef SM_VDSM1
	shadow *= shadow2DProj(shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r;
#endif

#ifdef SM_VDSM2
	shadow *= shadow2DProj(shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r;
	shadow *= shadow2DProj(shadowTexture1, gl_TexCoord[shadowTextureUnit1]).r;
#endif
	return shadow;
}

void main(void) 
{
	vec4 out_color = ov_geometry_color*texture2DArray(ov_billboard_texture, vec3(ov_geometry_texcoord, ov_geometry_tex_index));

	//add lighting 
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);

	
#ifdef BLT_ROTATED_QUAD
	//Create some fake normals
	float ny = 0;
	float nx = (ov_geometry_texcoord.x) * 2.0 - 1.0;
	float nz = sqrt(1.0 - (nx*nx));
	vec3 normal = normalize(vec3(4 * nx, ny, nz));// +ov_geometry_normal);
#else
	vec3 normal = normalize(ov_geometry_normal);
#endif
	float NdotL = max(dot(normal, light_dir), 0);

#ifndef BLT_ROTATED_QUAD //self shadows don't work well for rotated quads
	NdotL = ov_shadow(NdotL);
#endif

	out_color.xyz *= min(NdotL * gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz,2.0);
	
#if defined(FM_LINEAR) || defined(FM_EXP) || defined(FM_EXP2)
	float depth = ov_depth;//gl_FragCoord.z / gl_FragCoord.w;

#ifdef FM_LINEAR
	float fog_factor = (gl_Fog.end - depth) * gl_Fog.scale;
#endif

#ifdef FM_EXP
	float fog_factor = exp(-gl_Fog.density * depth);
#endif

#ifdef FM_EXP2
	float fog_factor = exp(-pow((gl_Fog.density * depth), 2.0));
#endif
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	out_color.xyz = mix(gl_Fog.color.xyz, out_color.xyz, fog_factor);
#endif
	
	gl_FragColor = out_color;
}