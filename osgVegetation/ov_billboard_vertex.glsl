#version 400

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;

in vec4 osg_Vertex;
in vec3 osg_Normal;
in vec4 osg_MultiTexCoord0;

out ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_out;

vec4 ov_applyTerrainElevation(vec4 pos, vec2 tex_coords);

void main()
{
	ov_out.TexCoord0 = osg_MultiTexCoord0.xy;
	ov_out.Position = ov_applyTerrainElevation(osg_Vertex, osg_MultiTexCoord0.xy);
	ov_out.Normal = osg_Normal;
}
