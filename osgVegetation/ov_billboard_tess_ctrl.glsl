#version 400 compatibility
layout(vertices = 3) out;

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

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform float ov_billboard_max_distance;
uniform float ov_billboard_density;
/*
bool ov_pointInViewFrustum(mat4 matrix, vec3 point)
{
    vec4 p;
    p = matrix * vec4(point,1);
    
    int out_of_bound[6] = int[6]( 0, 0, 0, 0, 0, 0 );
    out_of_bound[0] = int( p.x >  p.w );
    out_of_bound[1] = int( p.x < -p.w );
    out_of_bound[2] = int( p.y >  p.w );
    out_of_bound[3] = int( p.y < -p.w );
    out_of_bound[4] = int( p.z >  p.w );
    out_of_bound[5] = int( p.z < -p.w );
    
    return (out_of_bound[0] < 1 ) && ( out_of_bound[1] < 1 ) && ( out_of_bound[2] < 1 ) && ( out_of_bound[3] < 1 ) && ( out_of_bound[4] < 1 ) && ( out_of_bound[5] < 1 );
}
*/

#define ID gl_InvocationID
void main(){
	
	ov_out[ID].Position = ov_in[ID].Position;
	ov_out[ID].TexCoord0 = ov_in[ID].TexCoord0;
	ov_out[ID].Normal = ov_in[ID].Normal;

	if (ID == 0) 
	{
		float level = 1;
		vec4 p0 = osg_ModelViewMatrix * ov_in[0].Position;
		vec4 p1 = osg_ModelViewMatrix * ov_in[1].Position;
		vec4 p2 = osg_ModelViewMatrix * ov_in[2].Position;
		float min_z_dist =  min( min(p0.z, p1.z), p2.z); //if postive entire polygon behinde camera
		float min_dist =  min( min( length(p0.xyz), length(p1.xyz)), length(p2.xyz));
		
		float adjusted_max_dist = 10000;

		if(osg_ProjectionMatrix[3][3] == 0)
			adjusted_max_dist = ov_billboard_max_distance*max(osg_ProjectionMatrix[0][0],1);

		if(min_z_dist < 0 && min_dist < adjusted_max_dist)
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