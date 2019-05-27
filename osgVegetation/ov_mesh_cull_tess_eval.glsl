#version 400
layout(triangles, equal_spacing, ccw) in;
in vec4 ov_tc_position[];
in vec2 ov_tc_texcoord[];
out vec4 ov_te_position;
out vec2 ov_te_texcoord;

void main(){
	ov_te_position = (gl_TessCoord.x * ov_tc_position[0]) +
                 (gl_TessCoord.y * ov_tc_position[1]) +
                 (gl_TessCoord.z * ov_tc_position[2]);
				  
	ov_te_texcoord  = (gl_TessCoord.x * ov_tc_texcoord[0]) +
                (gl_TessCoord.y * ov_tc_texcoord[1]) +
                (gl_TessCoord.z * ov_tc_texcoord[2]);
	
}