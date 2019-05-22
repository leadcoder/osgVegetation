#version 400
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable

#pragma import_defines (OV_TERRAIN_COLOR_TEXTURE, OV_TERRAIN_SPLAT_MAPPING, OV_NOISE_TEXTURE)

#ifdef OV_TERRAIN_COLOR_TEXTURE
uniform sampler2D ov_color_texture;
#endif

#ifdef OV_NOISE_TEXTURE
uniform sampler2D ov_noise_texture;
#endif

uniform mat4 osg_ProjectionMatrix;

//detail params
//#ifdef OV_TERRAIN_SPLAT_MAPPING
uniform sampler2D ov_splat_texture;
uniform sampler2DArray ov_splat_detail_texture;
uniform vec4 ov_splat_detail_scale;
uniform int ov_splat_num_detail_textures;
uniform float ov_splat_max_distance;
uniform float ov_splat_color_modulate_ratio;
//#endif

vec4 ov_getSplatColor(vec2 splat_tex_coord, vec2 terrain_pos)
{
	vec4 splat_color = texture(ov_splat_texture, splat_tex_coord);

	//splat_color *= 2.0;
	splat_color = clamp(splat_color, 0, 1);

#ifdef OV_NOISE_TEXTURE
	float noise = texture2D(ov_noise_texture, terrain_pos * 0.002).x;
	float noise_2 = 1 - texture2D(ov_noise_texture, terrain_pos * 0.0001).x;
	noise =  noise_2*noise_2*noise * noise;
	noise = 1.0 - clamp(noise * 2.0, 0.0, 1.0);
	splat_color = splat_color * noise;
#endif
	return splat_color;
}

vec4 ov_detailTexturing(vec4 base_color, float depth, vec2 splat_tex_coord, vec2 terrain_pos)
{
	vec4 lc = ov_getSplatColor(splat_tex_coord, terrain_pos);

	vec4 detail_color = texture(ov_splat_detail_texture, vec3(terrain_pos * ov_splat_detail_scale.x, 0));

	for(int i = 1 ; i < ov_splat_num_detail_textures; i++)
	{
		vec4 di = texture(ov_splat_detail_texture, vec3(terrain_pos * ov_splat_detail_scale[i], i));
		detail_color = mix(detail_color, di, lc[i-1]);
	}

	//modulate with base color
	float mod_amount = 0.7;
	float mono_detail = (detail_color.x + detail_color.y + detail_color.z)/3.0;
	vec4 detail_modulate_color = 3.2 * base_color * mono_detail;
	detail_color = mix(detail_color, detail_modulate_color, ov_splat_color_modulate_ratio);
	
	//add some intensity variation
#ifdef OV_NOISE_TEXTURE
	float intensity = 1.0 + 0.3 * texture2D(ov_noise_texture, terrain_pos * 0.001).x;
	detail_color = intensity*detail_color;
#endif
	//fade to base color
	float end_fade = ov_splat_max_distance * max(osg_ProjectionMatrix[0][0], 1.0);
	float fade = clamp(depth / end_fade, 0.0, 1.0);

	vec4 out_color = 1.14 * mix(detail_color, base_color, fade);
	out_color.a = 1.0;
	return out_color;
}

vec4 ov_applyBC(vec4 color, float brightness, float contrast)
{
	// Apply contrast.
	color.rgb = ((color.rgb - 0.5) * max(contrast, 0)) + 0.5;
	// Apply brightness.
	color.rgb = color.rgb + brightness;
	//saturate
	color = clamp(color, 0.0, 1.0);
	return color;
}

vec4 ov_applyGamma(vec4 color, float value)
{
    return vec4(pow(abs(color.r), value),pow(abs(color.g), value),pow(abs(color.b), value), color.a);
}

vec4 ov_getTerrainColor(float depth, vec2 tex_coord0, vec2 terrain_pos)
{
	
#ifdef OV_TERRAIN_COLOR_TEXTURE
	vec4 out_color = texture2D(ov_color_texture, tex_coord0);
#else
	vec4 out_color = vec4(1,1,1,1);
#endif

	//out_color = ov_applyBC(out_color, 0.03, 0.8);
	//out_color = ov_applyGamma(out_color ,0.8);
#ifdef OV_TERRAIN_SPLAT_MAPPING
	out_color = ov_detailTexturing(out_color, depth, tex_coord0, terrain_pos);
#endif
	return out_color;
}