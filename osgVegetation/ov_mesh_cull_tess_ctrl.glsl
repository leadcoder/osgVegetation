#version 400
layout(vertices = 3) out;
uniform mat4 osg_ModelViewMatrix;
uniform float ov_TargetTriangleSide;

in ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_in[];

out ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_out[];


#define ID gl_InvocationID

float ov_rand(in vec2 seed)
{
    return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){
	ov_out[ID].Position = ov_in[ID].Position;
	ov_out[ID].Normal = ov_in[ID].Normal;
	ov_out[ID].TexCoord0 = ov_in[ID].TexCoord0;
	if (ID == 0) {
		//float level = 1;

		vec4 p0 = osg_ModelViewMatrix * ov_in[0].Position;
		vec4 p1 = osg_ModelViewMatrix * ov_in[1].Position;
		vec4 p2 = osg_ModelViewMatrix * ov_in[2].Position;
		
		float min_z_dist =  min( min(p0.z, p1.z), p2.z); //if postive entire polygon behinde camera

		if(min_z_dist < 0)
		{
			float l0 = length(p1.xyz - p2.xyz);
			float l1 = length(p0.xyz - p2.xyz);
			float l2 = length(p0.xyz - p1.xyz);
			float inner_factor = ov_TargetTriangleSide;//15.2;//ov_billboard_density;
			float outer_factor = ov_TargetTriangleSide;//15.2;//ov_billboard_density;
			float min_l = min(min(l0,l1),l2);
			float max_l = max(max(l0,l1),l2);
			if(min_l > ov_TargetTriangleSide)
			{
				gl_TessLevelOuter[0] = l0/outer_factor;
				gl_TessLevelOuter[1] = l1/outer_factor;
				gl_TessLevelOuter[2] = l2/outer_factor;
				gl_TessLevelInner[0] = ((l0+l1+l2)/3.0)/inner_factor;
			}
			else
			{
				float rand = ov_rand((ov_in[0].Position.xy + ov_in[1].Position.xy + ov_in[2].Position.xy)*0.33);
				float size_ratio = (min_l*min_l*0.5)/(ov_TargetTriangleSide*ov_TargetTriangleSide*0.5);
				float level = 0;
				if(size_ratio > rand)
					level = 1;
				gl_TessLevelOuter[0] = level;
				gl_TessLevelOuter[1] = level;
				gl_TessLevelOuter[2] = level;
				gl_TessLevelInner[0] = level;
			}
		}
		else
		{ 
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelInner[0] = 0;
		}


		//vec4 mv_pos1 = osg_ModelViewMatrix * ov_vertex_position[0];
		//vec4 mv_pos2 = osg_ModelViewMatrix * ov_vertex_position[1];
		//vec4 mv_pos3 = osg_ModelViewMatrix * ov_vertex_position[2];
		//float dist =  - min( min(mv_pos1.z, mv_pos2.z), mv_pos3.z);
		//if(dist < vegMaxDistance) {level = vegDensity;   }
		//else { level = 0;}
		
		//gl_TessLevelInner[0] = level;
        //gl_TessLevelOuter[0] = level;
        //gl_TessLevelOuter[1] = level;
        //gl_TessLevelOuter[2] = level;
	}
}