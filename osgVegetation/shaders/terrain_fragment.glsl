#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
varying vec2 texcoord;
//varying vec4 color;
uniform sampler2D baseTexture;

void main(void) 
{
	gl_FragColor = texture2D( baseTexture, texcoord);
	//gl_FragColor = vec4(1,0,1,1); 
}