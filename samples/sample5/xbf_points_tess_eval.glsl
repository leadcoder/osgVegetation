#version 400
layout(triangles, equal_spacing, ccw) in;

in vec4 ov_tc_position[];
in vec2 ov_tc_texcoord[];
out vec4 tePosition;
out vec2 teTexCoord;
//in float distScale[];

void main(){
	
	tePosition = ((gl_TessCoord.x * ov_tc_position[0]) +
                  (gl_TessCoord.y * ov_tc_position[1]) +
                  (gl_TessCoord.z * ov_tc_position[2]));
				  
	teTexCoord  = (gl_TessCoord.x * ov_tc_texcoord[0]) +
                 (gl_TessCoord.y * ov_tc_texcoord[1]) +
                 (gl_TessCoord.z * ov_tc_texcoord[2]);
}