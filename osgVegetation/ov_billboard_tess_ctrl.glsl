#version 400
layout(vertices = 3) out;

in vec4 ov_vertex_position[];
out vec4 ov_tc_position[];

in vec2 ov_vertex_texcoord[];
out vec2 ov_tc_texcoord[];

in vec3 ov_vertex_normal[];
out vec3 ov_tc_normal[];

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform float ov_billboard_max_distance;
uniform float ov_billboard_density;

#define ID gl_InvocationID
void main(){
	ov_tc_position[ID] = ov_vertex_position[ID];
	ov_tc_texcoord[ID] = ov_vertex_texcoord[ID];
	ov_tc_normal[ID] = ov_vertex_normal[ID];
	if (ID == 0) 
	{
		float level = 1;
		vec4 p0 = osg_ModelViewMatrix * ov_vertex_position[0];
		vec4 p1 = osg_ModelViewMatrix * ov_vertex_position[1];
		vec4 p2 = osg_ModelViewMatrix * ov_vertex_position[2];
		float dist =  -max( max(p0.z, p1.z), p2.z);
		
		float adjusted_max_dist = 10000;

		if(osg_ProjectionMatrix[3][3] == 0)
			adjusted_max_dist = ov_billboard_max_distance*max(osg_ProjectionMatrix[0][0],1);

		if(dist < adjusted_max_dist)
		{
			float l0 = length(p1.xyz - p2.xyz);
			float l1 = length(p0.xyz - p2.xyz);
			float l2 = length(p0.xyz - p1.xyz);
			float inner_factor = ov_billboard_density;
			float outer_factor = ov_billboard_density;
	    
			gl_TessLevelOuter[0] = l0/outer_factor;
			gl_TessLevelOuter[1] = l1/outer_factor;
			gl_TessLevelOuter[2] = l2/outer_factor;
			gl_TessLevelInner[0] = ((l0+l1+l2)/3.0)/inner_factor;
		}
		else 
		{ 
			
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;

			gl_TessLevelInner[0] = 0;
		}
		
		//gl_TessLevelInner[0] = level;
        //gl_TessLevelOuter[0] = level;
        //gl_TessLevelOuter[1] = level;
        //gl_TessLevelOuter[2] = level;
	}
}