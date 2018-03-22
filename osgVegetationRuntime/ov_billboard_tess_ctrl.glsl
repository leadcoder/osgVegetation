#version 400
layout(vertices = 3) out;
in vec4 ov_vertex_position[];
in vec2 ov_vertex_texcoord[];
out vec4 ov_tc_position[];
out vec2 ov_tc_texcoord[];
uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform float ov_billboard_max_distance;
uniform float ov_billboard_density;
#define ID gl_InvocationID
void main(){
	ov_tc_position[ID] = ov_vertex_position[ID];
	ov_tc_texcoord[ID] = ov_vertex_texcoord[ID];
	if (ID == 0) {
		float level = 1;
		vec4 mv_pos1 = osg_ModelViewMatrix * ov_vertex_position[0];
		vec4 mv_pos2 = osg_ModelViewMatrix * ov_vertex_position[1];
		vec4 mv_pos3 = osg_ModelViewMatrix * ov_vertex_position[2];
		float dist =  - max( max(mv_pos1.z, mv_pos2.z), mv_pos3.z);
		
		float adjusted_max_dist = 10000;

		if(osg_ProjectionMatrix[3][3] == 0)
			adjusted_max_dist = ov_billboard_max_distance*max(osg_ProjectionMatrix[0][0],1);

		if(dist < adjusted_max_dist)
		{
			float tri_size = length(ov_vertex_position[0].xyz - ov_vertex_position[1].xyz);
			tri_size += length(ov_vertex_position[0].xyz - ov_vertex_position[2].xyz);
			tri_size += length(ov_vertex_position[1].xyz - ov_vertex_position[2].xyz);
			level = 0.3 * tri_size/ov_billboard_density;
		}
		else 
		{ 
			level = 0;
		}
		
		gl_TessLevelInner[0] = level;
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
	}
}