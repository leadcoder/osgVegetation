
//varying vec4 ov_vertex_position;
//varying vec2 ov_vertex_texcoord;

out ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_out;

//uniform mat4 osg_ModelViewProjectionMatrix; 

void main(){
	ov_out.TexCoord0 = gl_MultiTexCoord0.xy;
	ov_out.Position = gl_Vertex;
    gl_Position = ov_out.Position;
    gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
}