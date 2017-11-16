#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
uniform sampler2DArray ov_billboard_texture;
varying vec2 ov_geometry_texcoord;
varying vec4 ov_geometry_color;
flat varying int ov_geometry_tex_index;

void main(void) 
{
	gl_FragColor = ov_geometry_color*texture2DArray(ov_billboard_texture, vec3(ov_geometry_texcoord, ov_geometry_tex_index));
}