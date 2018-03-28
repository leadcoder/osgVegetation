//#version 330 compatibility
varying vec4 ov_vertex_position;
varying vec2 ov_vertex_texcoord;
void main(void) 
{ 
	ov_vertex_texcoord = gl_MultiTexCoord0.xy;
    gl_Position = gl_Vertex;
	ov_vertex_position = gl_Vertex;
} 