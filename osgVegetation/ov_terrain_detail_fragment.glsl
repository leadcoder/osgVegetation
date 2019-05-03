#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable

uniform sampler2D ov_color_texture;
uniform sampler2D ov_land_cover_texture;
//uniform sampler2D ov_detail_texture0;
//uniform sampler2D ov_detail_texture1;
//uniform sampler2D ov_detail_texture2;
//uniform sampler2D ov_detail_texture3;
//uniform sampler2DArray ov_detail_texture;
uniform vec4 ov_detail_scale;
varying vec3 ov_normal;
varying vec2 ov_tex_coord0;
varying vec2 ov_tex_coord1;
varying float ov_depth;

vec3 ov_directionalLightShadow(vec3 normal);
vec3 ov_applyFog(vec3 color, float depth);
vec4 ov_detailTexturing(vec4 base_color, float depth);

void main(void) 
{
	float depth = ov_depth;
	//float depth = gl_FragCoord.z / gl_FragCoord.w;

	vec4 base_color = texture2D(ov_color_texture, ov_tex_coord0.xy);
	vec4 out_color = ov_detailTexturing(base_color, depth);

	//apply lighting and fog
	out_color.xyz *= ov_directionalLightShadow(normalize(ov_normal));
	out_color.xyz = ov_applyFog(out_color.xyz, depth);
	gl_FragColor = out_color;	
}