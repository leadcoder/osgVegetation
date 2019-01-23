//#version 140
varying vec3 ov_vertex_normal;
void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	ov_vertex_normal = normalize(gl_NormalMatrix * gl_Normal);
    gl_Position = gl_Vertex;
}