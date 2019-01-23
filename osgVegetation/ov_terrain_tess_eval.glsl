#version 400 compatibility
layout(triangles, equal_spacing, ccw) in;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
in vec3 ov_tc_normal[];
out vec3 ov_te_normal;
out float ov_depth;

void main(){

    vec4 mv_pos =	osg_ModelViewMatrix*((gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position));
	ov_depth = mv_pos.z;
	gl_Position = osg_ModelViewProjectionMatrix*((gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position));
				  
	gl_TexCoord[0]  = (gl_TessCoord.x * gl_in[0].gl_TexCoord[0]) +
                  (gl_TessCoord.y * gl_in[1].gl_TexCoord[0]) +
                  (gl_TessCoord.z * gl_in[2].gl_TexCoord[0]);

	ov_te_normal  = (gl_TessCoord.x * ov_tc_normal[0]) +
                  (gl_TessCoord.y * ov_tc_normal[1]) +
                  (gl_TessCoord.z * ov_tc_normal[2]);
	
}