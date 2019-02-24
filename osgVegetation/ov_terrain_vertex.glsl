//#version 140
#pragma import_defines (OV_TERRAIN_TESSELLATION)
#ifdef OV_TERRAIN_TESSELLATION
varying vec3 ov_vertex_normal;
#else
varying vec3 ov_normal;
//varying vec2 ov_tex_coord0;
//varying vec2 ov_tex_coord1;
varying float ov_depth;
void ov_setShadowTexCoords(vec4 mv_pos);
#endif

void main()
{
#ifdef OV_TERRAIN_TESSELLATION
	gl_Position = gl_Vertex;
	ov_vertex_normal = normalize(gl_NormalMatrix * gl_Normal);
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	ov_normal = normalize(gl_NormalMatrix * gl_Normal);
	//ov_tex_coord0 = gl_MultiTexCoord0.xy;
	//ov_tex_coord1 = gl_Vertex.xy;
	vec4 mv_pos = gl_ModelViewMatrix * gl_Vertex;
	ov_setShadowTexCoords(mv_pos);
	ov_depth = length(mv_pos.xyz);
#endif
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_Vertex;
}
