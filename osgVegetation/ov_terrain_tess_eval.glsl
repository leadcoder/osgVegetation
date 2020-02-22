#version 400

layout(triangles, equal_spacing, ccw) in;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;

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

void ov_setShadowTexCoords(vec4 mv_pos);
vec4 ov_applyTerrainElevation(vec4 pos, vec2 tex_coords);
vec3 ov_applyTerrainNormal(vec3 normal, vec2 tex_coords);

void main()
{
	ov_out.Position = (gl_TessCoord.x * ov_in[0].Position) + (gl_TessCoord.y * ov_in[1].Position) + (gl_TessCoord.z * ov_in[2].Position);
	ov_out.Normal  = (gl_TessCoord.x * ov_in[0].Normal) + (gl_TessCoord.y * ov_in[1].Normal) + (gl_TessCoord.z * ov_in[2].Normal);
	ov_out.TexCoord0 = (gl_TessCoord.x * ov_in[0].TexCoord0) + (gl_TessCoord.y * ov_in[1].TexCoord0) + (gl_TessCoord.z * ov_in[2].TexCoord0);

	ov_out.Normal = ov_applyTerrainNormal(ov_out.Normal, ov_out.TexCoord0);
	ov_out.Normal = osg_NormalMatrix * ov_out.Normal;
	ov_out.Position = ov_applyTerrainElevation(ov_out.Position, ov_out.TexCoord0);

	
	gl_Position = osg_ModelViewProjectionMatrix*ov_out.Position;
	
	vec4 mv_pos = osg_ModelViewMatrix * ov_out.Position;
	ov_setShadowTexCoords(mv_pos);
}