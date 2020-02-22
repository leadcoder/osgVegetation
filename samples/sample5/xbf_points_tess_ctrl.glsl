#version 400
layout(vertices = 3) out;
in vec4 ov_vertex_position[];
in vec2 ov_vertex_texcoord[];
out vec4 ov_tc_position[];
out vec2 ov_tc_texcoord[];
uniform mat4 osg_ModelViewMatrix;
uniform float vegMaxDistance;
uniform float ov_mesh_density;
#define ID gl_InvocationID
void main(){
	ov_tc_position[ID] = ov_vertex_position[ID];
	ov_tc_texcoord[ID] = ov_vertex_texcoord[ID];
	if (ID == 0) {
		float level = 2;
		vec4 mv_pos1 = osg_ModelViewMatrix * ov_vertex_position[0];
		vec4 mv_pos2 = osg_ModelViewMatrix * ov_vertex_position[1];
		vec4 mv_pos3 = osg_ModelViewMatrix * ov_vertex_position[2];
		//float dist =  - max( max(mv_pos1.z, mv_pos2.z), mv_pos3.z);
		float dist =  min( min(length(mv_pos1.xyz), length(mv_pos2.xyz)), length(mv_pos3.xyz));
		//float vegMaxDistance = 2000;
		if(dist > 0.1 && dist < vegMaxDistance) 
		{
			float tri_size = length(ov_vertex_position[0].xyz - ov_vertex_position[1].xyz);
			tri_size += length(ov_vertex_position[0].xyz - ov_vertex_position[2].xyz);
			tri_size += length(ov_vertex_position[1].xyz - ov_vertex_position[2].xyz);
			level = 0.3 * tri_size / ov_mesh_density;
			//level = vegDensity;   
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