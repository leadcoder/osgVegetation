#version 400
layout(triangles, equal_spacing, ccw) in;

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
} ov_out;

void main()
{
	ov_out.Position = (gl_TessCoord.x * ov_in[0].Position) +
                      (gl_TessCoord.y * ov_in[1].Position) +
                      (gl_TessCoord.z * ov_in[2].Position);
	
     ov_out.Normal  = (gl_TessCoord.x * ov_in[0].Normal) +
                      (gl_TessCoord.y * ov_in[1].Normal) +
                      (gl_TessCoord.z * ov_in[2].Normal);

	ov_out.TexCoord0 = (gl_TessCoord.x * ov_in[0].TexCoord0) +
                       (gl_TessCoord.y * ov_in[1].TexCoord0) +
                       (gl_TessCoord.z * ov_in[2].TexCoord0);
}