//#version 140
#pragma import_defines (OV_TERRAIN_TESSELLATION, OV_TERRAIN_ELEVATION_TEXTURE)
#ifdef OV_TERRAIN_TESSELLATION
varying vec3 ov_vertex_normal;
#else
varying vec3 ov_normal;
varying float ov_depth;
void ov_setShadowTexCoords(vec4 mv_pos);
#endif

#ifdef OV_TERRAIN_ELEVATION_TEXTURE
uniform sampler2D ov_elevation_texture;
#endif

void main()
{
#ifdef OV_TERRAIN_TESSELLATION
	gl_Position = gl_Vertex;
	ov_vertex_normal = normalize(gl_NormalMatrix * gl_Normal);
#else
	vec4 pos = gl_Vertex;
#ifdef OV_TERRAIN_ELEVATION_TEXTURE
	//offset and scale texture coordiantes to match pixel center to vertex pos
	ivec2 elev_texture_size = textureSize(ov_elevation_texture,0);
	vec2 elev_texel_size = vec2(1.0/ float(elev_texture_size.x), 1.0/float(elev_texture_size.y)); 
	vec2 tex_coord_scale = vec2(1.0, 1.0) - elev_texel_size; //strech texture by on texel
	vec2 tex_coord_offset = 0.5 * elev_texel_size; //offset texture to match pixel center with vertex pos
	pos.z = texture2D(ov_elevation_texture, (tex_coord_scale * gl_MultiTexCoord0.xy) + tex_coord_offset).x;
#endif
	gl_Position = gl_ModelViewProjectionMatrix * pos;
	ov_normal = normalize(gl_NormalMatrix * gl_Normal);
	//ov_tex_coord0 = gl_MultiTexCoord0.xy;
	//ov_tex_coord1 = gl_Vertex.xy;
	vec4 mv_pos = gl_ModelViewMatrix * pos;
	ov_setShadowTexCoords(mv_pos);
	ov_depth = length(mv_pos.xyz);
#endif
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_Vertex;
}
