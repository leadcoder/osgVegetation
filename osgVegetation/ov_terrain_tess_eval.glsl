#version 400 compatibility
#pragma import_defines (OV_TERRAIN_ELEVATION_TEXTURE)
layout(triangles, equal_spacing, ccw) in;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
in vec3 ov_tc_normal[];
out vec3 ov_normal;
out float ov_depth;
void ov_setShadowTexCoords(vec4 mv_pos);

#ifdef OV_TERRAIN_ELEVATION_TEXTURE
uniform sampler2D ov_elevation_texture;
#endif

void main(){

	vec4 pos = (gl_TessCoord.x * gl_in[0].gl_Position) +
               (gl_TessCoord.y * gl_in[1].gl_Position) +
               (gl_TessCoord.z * gl_in[2].gl_Position);
				  
	 vec4 tex_coord_0 = (gl_TessCoord.x * gl_in[0].gl_TexCoord[0]) +
                  (gl_TessCoord.y * gl_in[1].gl_TexCoord[0]) +
                  (gl_TessCoord.z * gl_in[2].gl_TexCoord[0]);

#ifdef OV_TERRAIN_ELEVATION_TEXTURE
	pos.z = texture2D(ov_elevation_texture, tex_coord_0.xy).x;
#endif
	gl_Position = osg_ModelViewProjectionMatrix*pos;
	gl_TexCoord[0] = tex_coord_0;

	gl_TexCoord[1]  = (gl_TessCoord.x * gl_in[0].gl_TexCoord[1]) +
                  (gl_TessCoord.y * gl_in[1].gl_TexCoord[1]) +
                  (gl_TessCoord.z * gl_in[2].gl_TexCoord[1]);

	ov_normal  = (gl_TessCoord.x * ov_tc_normal[0]) +
                  (gl_TessCoord.y * ov_tc_normal[1]) +
                  (gl_TessCoord.z * ov_tc_normal[2]);

	vec4 mv_pos = osg_ModelViewMatrix * pos;
	ov_depth = length(mv_pos.xyz);

	ov_setShadowTexCoords(mv_pos);
	
}