#version 400 compatibility

in vec4 osg_Vertex;
in vec3 osg_Normal;
in vec4 osg_MultiTexCoord0;

out ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_out;

void main()
{
	ov_out.Position = osg_Vertex;
    //TODO: investigate why we cant use osg_Normal and osg_MultiTexCoord0 here like in ov_billboard_vertex.glsl
	ov_out.Normal = gl_Normal; 
    ov_out.TexCoord0 = gl_MultiTexCoord0.xy;
	
}