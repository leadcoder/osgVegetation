#version 400
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

#define ID gl_InvocationID

void main()
{
	ov_out[ID].Position = ov_in[ID].Position;
	ov_out[ID].TexCoord0 = ov_in[ID].TexCoord0;
	ov_out[ID].Normal = ov_in[ID].Normal;
	
	if (ID == 0) {
		float level = 8;
#if 1
		vec4 p0 = osg_ModelViewMatrix * ov_in[0].Position;
		vec4 p1 = osg_ModelViewMatrix * ov_in[1].Position;
		vec4 p2 = osg_ModelViewMatrix * ov_in[2].Position;
		float l0 = length(p1.xyz - p2.xyz);
		float l1 = length(p0.xyz - p2.xyz);
		float l2 = length(p0.xyz - p1.xyz);
		float density = 1.0/4.0;//0.1;
		float inner_factor = 2.0/density;
		float outer_factor = 2.0/density;
	    
		gl_TessLevelOuter[0] = l0/outer_factor;
        gl_TessLevelOuter[1] = l1/outer_factor;
        gl_TessLevelOuter[2] = l2/outer_factor;

		gl_TessLevelInner[0] = ((l0+l1+l2)/3.0)/inner_factor;
#endif
		gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
		gl_TessLevelInner[0] = (level + level + level)/3;
	}
}