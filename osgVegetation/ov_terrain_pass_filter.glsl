#version 330 
#pragma import_defines (OV_TERRAIN_COLOR_TEXTURE, OV_SPLAT_FILTER(splat_color), OV_COLOR_FILTER(base_color))

#ifdef OV_TERRAIN_COLOR_TEXTURE
uniform sampler2D ov_color_texture;
#endif

#ifdef OV_SPLAT_FILTER
vec4 ov_getSplatColor(vec2 splat_tex_coord, vec2 terrain_pos);
#endif
bool ov_passFilter(vec3 terrain_pos, vec3 terrain_normal, vec2 terrain_texcoord)
{
#ifdef OV_SPLAT_FILTER
	vec4 lc_color = ov_getSplatColor(terrain_texcoord, terrain_pos.xy);
	OV_SPLAT_FILTER(lc_color)
#endif
	//sample terrain color
#ifdef OV_COLOR_FILTER
#ifdef OV_TERRAIN_COLOR_TEXTURE
	vec4 base_color = texture2D(ov_color_texture, terrain_texcoord);
	OV_COLOR_FILTER(base_color);
#endif
#endif
	//Slope filter
	if(dot(normalize(terrain_normal), vec3(0,0,1)) < 0.9)
		return false;
	return true;
}