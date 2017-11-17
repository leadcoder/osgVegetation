#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable

uniform sampler2D ov_color_texture;
uniform sampler2D ov_land_cover_texture;
uniform sampler2D ov_detail_texture0;
uniform sampler2D ov_detail_texture1;

void main(void) 
{
	vec4 base_color = texture2D(ov_color_texture, gl_TexCoord[0].xy);
	vec4 lc = texture2D(ov_land_cover_texture, gl_TexCoord[0].xy);
	vec4 d0 = texture2D(ov_detail_texture0, gl_TexCoord[0].xy*11);
	vec4 d1 = texture2D(ov_detail_texture1, gl_TexCoord[0].xy*7);
	d0.w = (d0.x + d0.y + d0.z)/3.0;
	d1.w = (d1.x + d1.y + d1.z)/3.0;
	vec4 out_color = 2.2*base_color*d0.w*(1.0 - lc.x) + 2*base_color*d1.w*lc.x;
	out_color.a = 1;
	gl_FragColor = out_color;	
}