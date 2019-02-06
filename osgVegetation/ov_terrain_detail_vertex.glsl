//#version 140
//uniform mat4 osg_ModelViewProjectionMatrix;
varying vec3 ov_normal;
varying vec2 ov_tex_coord0;
varying vec2 ov_tex_coord1;
varying float ov_depth;

void ov_setShadowTexCoords(vec4 mv_pos);

void main() {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	ov_normal = normalize(gl_NormalMatrix * gl_Normal);
	ov_tex_coord0 = gl_MultiTexCoord0.xy;
	ov_tex_coord1 = gl_Vertex.xy;
	vec4 mv_pos = gl_ModelViewMatrix * gl_Vertex;
	ov_setShadowTexCoords(mv_pos);
	ov_depth = length(mv_pos.xyz);
}