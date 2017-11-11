#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
varying vec2 texcoord;
//varying vec4 color;
uniform sampler2D baseTexture;
uniform sampler2D landCoverTexture;
uniform sampler2D detailTexture0;
uniform sampler2D detailTexture1;

void main(void) 
{
	vec4 base_color = texture2D( baseTexture, gl_TexCoord[0].xy);
	vec4 lc = texture2D( landCoverTexture, gl_TexCoord[0].xy);
	vec4 d0 = texture2D( detailTexture0, gl_TexCoord[0].xy*7);
	vec4 d1 = texture2D( detailTexture1, gl_TexCoord[0].xy*7);
	d0.w = (d0.x + d0.y + d0.z)/3.0;
	d1.w = (d1.x + d1.y + d1.z)/3.0;
	vec4 out_color = 2.2*base_color*d0.w*(1.0 - lc.x) + 2*base_color*d1.w*lc.x;
	out_color.a = 1;
	gl_FragColor = out_color;	
	//gl_FragColor = texture2D( baseTexture, gl_TexCoord[0].xy);
	//gl_FragColor = vec4(1,0,1,1); 
}