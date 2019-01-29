#version 400
layout(triangles, equal_spacing, ccw) in;
in vec4 ov_tc_position[];

in vec2 ov_tc_texcoord[];
out vec2 ov_te_texcoord;

in vec3 ov_tc_normal[];
out vec3 ov_te_normal;

uniform mat4 osg_ModelViewProjectionMatrix;
 
void main(){
	
	gl_Position = (gl_TessCoord.x * ov_tc_position[0]) +
                  (gl_TessCoord.y * ov_tc_position[1]) +
                  (gl_TessCoord.z * ov_tc_position[2]);
				  
	ov_te_texcoord = (gl_TessCoord.x * ov_tc_texcoord[0]) +
                  (gl_TessCoord.y * ov_tc_texcoord[1]) +
                  (gl_TessCoord.z * ov_tc_texcoord[2]);
	
	ov_te_normal  = (gl_TessCoord.x * ov_tc_normal[0]) +
                  (gl_TessCoord.y * ov_tc_normal[1]) +
                  (gl_TessCoord.z * ov_tc_normal[2]);
}