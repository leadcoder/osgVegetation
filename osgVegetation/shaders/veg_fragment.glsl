#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
uniform sampler2DArray vegTexture; 
varying vec2 otexcoord;
varying vec4 color;
flat varying int veg_type;

void main(void) 
{
	
	gl_FragColor = color*texture2DArray(vegTexture, vec3(otexcoord, veg_type)); 
//    gl_FragColor = texture2D( vegTexture, otexcoord); 
}