#version 400
layout(vertices = 3) out;
uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ProjectionMatrix;
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

bool ov_FilterTriangle()
{
	vec4 vertices[3];
	vertices[0] = osg_ModelViewProjectionMatrix * ov_in[0].Position;
	vertices[1] = osg_ModelViewProjectionMatrix * ov_in[1].Position;
	vertices[2] = osg_ModelViewProjectionMatrix * ov_in[2].Position;

	int verticesInFrontOfNearPlane = 0;
	
	for (uint i = 0U; i < 3U; i++)
	{
		if (vertices[i].w < 0.0f)
		{
			++verticesInFrontOfNearPlane;

			// Flip the w so that any triangle that straddles the plane won't be projected onto
			// two sides of the screen
			vertices[i].w *= (-1.0f);
		}
		// Transform vertices[i].xy into the normalized 0..1 screen space
		// this is for the following stages ...
		vertices[i].xy /= vertices[i].w * 2.0f;
		vertices[i].xy += vec2(0.5f, 0.5f);
	}
	if (verticesInFrontOfNearPlane == 3)
		return true;

	float minx = min(min(vertices[0].x, vertices[1].x), vertices[2].x);
	float miny = min(min(vertices[0].y, vertices[1].y), vertices[2].y);
	float maxx = max(max(vertices[0].x, vertices[1].x), vertices[2].x);
	float maxy = max(max(vertices[0].y, vertices[1].y), vertices[2].y);

	if ((maxx < 0.0f) || (maxy < -0.7f) || (minx > 1.0f) || (miny > 1.0f))
		return true;
	return false;
}

void main(){
	ov_out[ID].Position = ov_in[ID].Position;
	ov_out[ID].Normal = ov_in[ID].Normal;
	ov_out[ID].TexCoord0 = ov_in[ID].TexCoord0;
	if (ID == 0) 
	{
		if(!ov_FilterTriangle())
		{
			vec4 p0 = osg_ModelViewMatrix * ov_in[0].Position;
			vec4 p1 = osg_ModelViewMatrix * ov_in[1].Position;
			vec4 p2 = osg_ModelViewMatrix * ov_in[2].Position;
		
			float l0 = length(p1.xyz - p2.xyz);
			float l1 = length(p0.xyz - p2.xyz);
			float l2 = length(p0.xyz - p1.xyz);
			float inner_factor = ov_TargetTriangleSide;
			float outer_factor = ov_TargetTriangleSide;
			float min_l = min(min(l0,l1),l2);
			float max_l = max(max(l0,l1),l2);

			//Check if triangle need tessellation, we use the minimum triangle side for now
			if(min_l > ov_TargetTriangleSide) 
			{
				gl_TessLevelOuter[0] = l0/outer_factor;
				gl_TessLevelOuter[1] = l1/outer_factor;
				gl_TessLevelOuter[2] = l2/outer_factor;
				gl_TessLevelInner[0] = ((l0+l1+l2)/3.0)/inner_factor;
			}
			else //else already have a triangle that is smaller than our target size
			{
				//if we always pass triangles here we could get to dense vegetation
				//To avoid this we randomly pass triangles, the probability depends on area ratio. 
				//We approximate this by assuming equilateral triangle for now. 
				float norm_rand = ov_rand((ov_in[0].Position.xy + ov_in[1].Position.xy + ov_in[2].Position.xy)*0.33);
				float size_ratio = (min_l*min_l)/(ov_TargetTriangleSide*ov_TargetTriangleSide);
				float tess_level = size_ratio > norm_rand ? 1 : 0;
				
				gl_TessLevelOuter[0] = tess_level;
				gl_TessLevelOuter[1] = tess_level;
				gl_TessLevelOuter[2] = tess_level;
				gl_TessLevelInner[0] = tess_level;
			}
		}
		else //triangle behinde camera
		{ 
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelInner[0] = 0;
		}
	}
}