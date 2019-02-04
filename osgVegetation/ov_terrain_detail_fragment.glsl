#version 120
//#extension GL_EXT_gpu_shader4 : enable
//#extension GL_EXT_texture_array : enable

uniform sampler2D ov_color_texture;
uniform sampler2D ov_land_cover_texture;
uniform sampler2D ov_detail_texture0;
uniform sampler2D ov_detail_texture1;
uniform sampler2D ov_detail_texture2;
uniform sampler2D ov_detail_texture3;
uniform vec4 ov_detail_scale;
varying vec3 ov_normal;
varying vec2 ov_tex_coord0;
varying vec2 ov_tex_coord1;
varying float ov_depth;

vec3 ov_directional_light_shadow(vec3 normal);
vec3 ov_apply_fog(vec3 color, float depth);

void main(void) 
{
	float depth = ov_depth;//gl_FragCoord.z / gl_FragCoord.w;

	vec4 base_color = texture2D(ov_color_texture, ov_tex_coord0.xy);
	vec4 lc = texture2D(ov_land_cover_texture, ov_tex_coord0.xy);
	
	vec4 d0 = texture2D(ov_detail_texture0, ov_tex_coord1.xy*ov_detail_scale.x);
	vec4 d1 = texture2D(ov_detail_texture1, ov_tex_coord1.xy*ov_detail_scale.y);
	vec4 d2 = texture2D(ov_detail_texture2, ov_tex_coord1.xy*ov_detail_scale.z);
	vec4 d3 = texture2D(ov_detail_texture3, ov_tex_coord1.xy*ov_detail_scale.w);
	
	//d0.w = (d0.x + d0.y + d0.z)/3.0;
	//d1.w = (d1.x + d1.y + d1.z)/3.0;
	//d2.w = (d2.x + d2.y + d2.z)/3.0;
	//d3.w = (d3.x + d3.y + d3.z)/3.0;

	vec4 detail_color = mix(d0, d1, lc.x);
	detail_color = mix(detail_color, d2, lc.y);
	detail_color = mix(detail_color, d3, lc.z);
	detail_color.w = (detail_color.x + detail_color.y + detail_color.z)/3.0;
	vec4 detail_base_color = 2 * base_color * detail_color.wwww;
	vec4 out_color = mix(detail_color, detail_base_color, 0.3);
	//2 * base_color * detail_color.wwww + 0.5*detail_color;

	float end_fade = 500.0*max(gl_ProjectionMatrix[0][0], 1);

	float fade = clamp(depth / end_fade, 0.0, 1);
	out_color = 1.3*mix(out_color, base_color, fade);
	out_color.a = 1;

	/*vec4 d0 = texture2D(ov_detail_texture0, ov_tex_coord0.xy*22);
	vec4 d1 = texture2D(ov_detail_texture1, ov_tex_coord0.xy*9);
	d0.w = (d0.x + d0.y + d0.z)/3.0;
	d1.w = (d1.x + d1.y + d1.z)/3.0;
	vec4 out_color = 2.2*base_color*d0.w*(1.0 - lc.x) + 2*base_color*d1.w*lc.x;
	out_color.a = 1;
	vec4 detail_color = d0*(1.0 - lc.x) + d1*lc.x;

	float fade = clamp(depth / 500.0, 0.5, 1);
	out_color = mix(detail_color, out_color, fade);
	out_color.a = 1;
	*/

	//apply lighting and fog
	out_color.xyz *= ov_directional_light_shadow(normalize(ov_normal));
	out_color.xyz = ov_apply_fog(out_color.xyz, depth);
	gl_FragColor = out_color;	
}