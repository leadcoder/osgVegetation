//#version 140
uniform mat4 osg_ModelViewProjectionMatrix;
varying vec3 ov_normal;
varying vec2 ov_tex_coord0;
void main() {
	gl_Position = osg_ModelViewProjectionMatrix* gl_Vertex;
	ov_normal = normalize(gl_NormalMatrix * gl_Normal);
	ov_tex_coord0 = gl_MultiTexCoord0.xy;
}