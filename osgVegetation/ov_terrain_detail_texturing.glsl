#version 400 compatibility
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable

uniform sampler2D ov_land_cover_texture;
uniform sampler2DArray ov_detail_texture;
uniform sampler2D ov_noise_texture;
uniform vec4 ov_detail_scale;
uniform int ov_num_detail_textures;

vec4 ov_getSplatColor()
{
	vec4 splat_color = texture(ov_land_cover_texture, gl_TexCoord[0].xy);
	vec4 noise = texture2D(ov_noise_texture, gl_TexCoord[1].xy * 0.002);
	noise.x = noise.x * noise.x;
	noise.x = 1.0 - clamp(noise.x * 2.0, 0.0, 1.0);

	splat_color *= 2.0;
	splat_color = clamp(splat_color, 0, 1);
	splat_color = splat_color * noise.x;
	return splat_color;
}

vec4 ov_detailTexturing(vec4 base_color, float depth)
{
	vec4 splat_color = ov_getSplatColor();

	vec4 detail_color = texture(ov_detail_texture, vec3(gl_TexCoord[1].xy * ov_detail_scale.x, 0));

	for(int i = 1 ; i < ov_num_detail_textures; i++)
	{
		vec4 di = texture(ov_detail_texture, vec3(gl_TexCoord[1].xy * ov_detail_scale[i], i));
		detail_color = mix(detail_color, di, splat_color[i-1]);
	}

	//modulate with base color
	float mono_detail = (detail_color.x + detail_color.y + detail_color.z)/3.0;
	vec4 detail_modulate_color = 2.0 * base_color * mono_detail;
	detail_color = mix(detail_color, detail_modulate_color, 0.1);
	
	//add some intensity variation
	float intensity = 1.0 + 0.3*texture2D(ov_noise_texture, gl_TexCoord[1].xy * 0.004).x;
	detail_color = intensity*detail_color;
	
	//fade to base color
	float end_fade = 10.0 * max(gl_ProjectionMatrix[0][0], 1.0);
	float fade = clamp(depth / end_fade, 0.0, 1.0);

	vec4 out_color = 1.14 * mix(detail_color, base_color, fade);
	out_color.a = 1.0;
	return out_color;
}
