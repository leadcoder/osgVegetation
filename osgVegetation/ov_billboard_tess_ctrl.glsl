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
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform float ov_billboard_max_distance;
uniform float ov_billboard_density;

bool ov_InView(in vec4 vertex_view)
{
    vec4 clip = osg_ProjectionMatrix * vertex_view;
    clip.xyz /= clip.w;
    //return abs(clip.x) <= 1.01 && clip.y < 1.0;
	//return abs(clip.z) > 1.0;
	return abs(clip.x) <= 1.01 && clip.y < 1.0 && abs(clip.z) < 1.0;
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

#define ID gl_InvocationID
void main(){
	
	ov_out[ID].Position = ov_in[ID].Position;
	ov_out[ID].TexCoord0 = ov_in[ID].TexCoord0;
	ov_out[ID].Normal = ov_in[ID].Normal;

	if (ID == 0) 
	{
		float level = 1;
		
		bool inside_view = !ov_FilterTriangle();
		if(inside_view)
		{
			vec4 p0 = osg_ModelViewMatrix * ov_in[0].Position;
			vec4 p1 = osg_ModelViewMatrix * ov_in[1].Position;
			vec4 p2 = osg_ModelViewMatrix * ov_in[2].Position;
		
			float min_dist =  min( min( length(p0.xyz), length(p1.xyz)), length(p2.xyz));
			float adjusted_max_dist = 10000;
			if(osg_ProjectionMatrix[3][3] == 0)
				adjusted_max_dist = ov_billboard_max_distance*max(osg_ProjectionMatrix[0][0],1);

			if(min_dist < adjusted_max_dist)
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
		}
		else 
		{ 
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelInner[0] = 0;
		}
	}
}