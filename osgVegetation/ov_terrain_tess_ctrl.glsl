#version 400
layout(vertices = 3) out;
uniform mat4 osg_ModelViewMatrix;
in vec3 ov_vertex_normal[];
out vec3 ov_tc_normal[];
#define ID gl_InvocationID

void main(){
	gl_out[ID].gl_Position =  gl_in[ID].gl_Position;
	gl_out[ID].gl_TexCoord[0] = gl_in[ID].gl_TexCoord[0];
	gl_out[ID].gl_TexCoord[1] = gl_in[ID].gl_TexCoord[1];
	ov_tc_normal[ID] = ov_vertex_normal[ID];
	if (ID == 0) {
		float level = 4;
		vec4 p0 = osg_ModelViewMatrix*gl_in[0].gl_Position;
		vec4 p1 = osg_ModelViewMatrix*gl_in[1].gl_Position;
		vec4 p2 = osg_ModelViewMatrix*gl_in[2].gl_Position;
		float l0 = length(p1.xyz - p2.xyz);
		float l1 = length(p0.xyz - p2.xyz);
		float l2 = length(p0.xyz - p1.xyz);
		float density = 1.0/10;//0.1;
		float inner_factor = 2.0/density;
		float outer_factor = 2.0/density;
	    
		gl_TessLevelOuter[0] = l0/outer_factor;
        gl_TessLevelOuter[1] = l1/outer_factor;
        gl_TessLevelOuter[2] = l2/outer_factor;

		gl_TessLevelInner[0] = ((l0+l1+l2)/3.0)/inner_factor;
	}
}