#version 400 compatibility
#pragma import_defines (OV_TERRAIN_ELEVATION_TEXTURE)

#ifdef OV_TERRAIN_ELEVATION_TEXTURE
uniform sampler2D ov_elevation_texture;
#endif

float ov_getTerrainElevation(vec2 tex_coords)
{
	float elevation = 0.0;
#ifdef OV_TERRAIN_ELEVATION_TEXTURE
	//offset and scale texture coordiantes to match pixel center to vertex pos
	ivec2 elev_texture_size = textureSize(ov_elevation_texture,0);
	vec2 elev_texel_size = vec2(1.0/ float(elev_texture_size.x), 1.0/float(elev_texture_size.y)); 
	vec2 tex_coord_scale = vec2(1.0, 1.0) - elev_texel_size; //strech texture by on texel
	vec2 tex_coord_offset = 0.5 * elev_texel_size; //offset texture to match pixel center with vertex pos
	elevation = texture2D(ov_elevation_texture, (tex_coord_scale * tex_coords) + tex_coord_offset).x;
#endif
	return elevation;
}

vec4 ov_applyTerrainElevation(vec4 pos, vec2 tex_coords)
{
	#ifdef OV_TERRAIN_ELEVATION_TEXTURE
	pos.z = ov_getTerrainElevation(tex_coords);
	#endif
	return pos;
}

vec3 ov_getTerrainNormal(vec2 tex_coords)
{
	vec3 normal = vec3(0,0,1);
	#ifdef OV_TERRAIN_ELEVATION_TEXTURE
		ivec2 elev_texture_size = textureSize(ov_elevation_texture,0);
		vec2 elev_texel_size = vec2(1.0/ float(elev_texture_size.x), 1.0/float(elev_texture_size.y)); 
		vec2 tex_coord_scale = vec2(1.0, 1.0) - elev_texel_size; //strech texture by on texel
		vec2 tex_coord_offset = 0.5 * elev_texel_size; //offset texture to match pixel center with vertex pos
		vec2 tex_coords_final = tex_coord_scale * tex_coords + tex_coord_offset;

		float TerrainSize = 8000;
		float quad_size  = TerrainSize/512;
		vec3 pixel_size = vec3(1.0/float(elev_texture_size.x), -1.0/float(elev_texture_size.y), 0);// / textureSize(ov_elevation_texture, 0).xxx;
		float u0 = texture(ov_elevation_texture, tex_coords_final + pixel_size.yz).x;
		float u1 = texture(ov_elevation_texture, tex_coords_final + pixel_size.xz).x;
		float v0 = texture(ov_elevation_texture, tex_coords_final + pixel_size.zy).x;
		float v1 = texture(ov_elevation_texture, tex_coords_final + pixel_size.zx).x;
		vec3 tangent = normalize(vec3(quad_size, 0, u1 - u0));
		vec3 binormal = normalize(vec3(0, quad_size, v1 - v0));
		normal = normalize(cross(tangent, binormal));

	#endif
	return normal;
}

vec3 ov_applyTerrainNormal(vec3 normal, vec2 tex_coords)
{
	#ifdef OV_TERRAIN_ELEVATION_TEXTURE
	normal = ov_getTerrainNormal(tex_coords);
	#endif
	return normal;
}