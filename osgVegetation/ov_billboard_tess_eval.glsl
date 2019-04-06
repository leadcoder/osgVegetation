#version 400
//#pragma import_defines (OV_TERRAIN_ELEVATION_TEXTURE)
layout(triangles, equal_spacing, ccw) in;
in vec4 ov_tc_position[];

in vec2 ov_tc_texcoord[];
out vec2 ov_te_texcoord;

in vec3 ov_tc_normal[];
out vec3 ov_te_normal;

uniform mat4 osg_ModelViewProjectionMatrix;

//#ifdef OV_TERRAIN_ELEVATION_TEXTURE
//uniform sampler2D ov_elevation_texture;
//#endif

void main(){
				  
	vec4 pos = (gl_TessCoord.x * ov_tc_position[0]) +
               (gl_TessCoord.y * ov_tc_position[1]) +
               (gl_TessCoord.z * ov_tc_position[2]);

	vec2 tex_coord_0 = (gl_TessCoord.x * ov_tc_texcoord[0]) +
                  (gl_TessCoord.y * ov_tc_texcoord[1]) +
                  (gl_TessCoord.z * ov_tc_texcoord[2]);

//#ifdef OV_TERRAIN_ELEVATION_TEXTURE
//	pos.z = texture2D(ov_elevation_texture, tex_coord_0.xy).x;
//#endif
 
	gl_Position = pos;
	ov_te_texcoord = tex_coord_0; 
	
	ov_te_normal  = (gl_TessCoord.x * ov_tc_normal[0]) +
                  (gl_TessCoord.y * ov_tc_normal[1]) +
                  (gl_TessCoord.z * ov_tc_normal[2]);
}