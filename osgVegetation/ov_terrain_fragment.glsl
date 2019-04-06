#version 120
#pragma import_defines (OV_TERRAIN_COLOR_TEXTURE, OV_TERRAIN_DETAIL_TEXTURING)
#ifdef OV_TERRAIN_COLOR_TEXTURE
uniform sampler2D ov_color_texture;
#endif
in vec3 ov_normal;
in float ov_depth;

vec3 ov_directionalLightShadow(vec3 normal);
vec3 ov_applyFog(vec3 color, float depth);
vec4 ov_detailTexturing(vec4 base_color, float depth);

void main(void) 
{
#ifdef OV_TERRAIN_COLOR_TEXTURE
	vec4 out_color = texture2D(ov_color_texture, gl_TexCoord[0].xy);
#else
	vec4 out_color = vec4(1,1,1,1);
#endif
#ifdef OV_TERRAIN_DETAIL_TEXTURING
	out_color = ov_detailTexturing(out_color, ov_depth);
#endif
	//apply lighting and fog
	out_color.xyz *= ov_directionalLightShadow(normalize(ov_normal));
	out_color.xyz = ov_applyFog(out_color.xyz, ov_depth);
	gl_FragColor = out_color;	
}