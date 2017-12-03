#version 120
//#extension GL_EXT_gpu_shader4 : enable
//#extension GL_EXT_texture_array : enable
#pragma import_defines (FM_LINEAR,FM_EXP,FM_EXP2)

uniform sampler2D ov_color_texture;
uniform sampler2D ov_land_cover_texture;
uniform sampler2D ov_detail_texture0;
uniform sampler2D ov_detail_texture1;
varying vec3 ov_normal;
varying vec2 ov_tex_coord0;
void main(void) 
{
	vec4 base_color = texture2D(ov_color_texture, ov_tex_coord0.xy);
	vec4 lc = texture2D(ov_land_cover_texture, ov_tex_coord0.xy);
	vec4 d0 = texture2D(ov_detail_texture0, ov_tex_coord0.xy*22);
	vec4 d1 = texture2D(ov_detail_texture1, ov_tex_coord0.xy*9);
	d0.w = (d0.x + d0.y + d0.z)/3.0;
	d1.w = (d1.x + d1.y + d1.z)/3.0;
	vec4 out_color = 2.2*base_color*d0.w*(1.0 - lc.x) + 2*base_color*d1.w*lc.x;
	out_color.a = 1;

	float depth = gl_FragCoord.z / gl_FragCoord.w;
	
	vec4 detail_color = d0*(1.0 - lc.x) + d1*lc.x;

	float fade = clamp(depth / 500.0, 0.5, 1);
	out_color = mix(detail_color, out_color, fade);
	out_color.a = 1;

	//apply lighting 
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	vec3 normal = normalize(ov_normal);
	float NdotL = max(dot(normal, light_dir), 0);
	out_color.xyz *= min(NdotL * gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz, 2.0);


#if defined(FM_LINEAR) || defined(FM_EXP) || defined(FM_EXP2)
	

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