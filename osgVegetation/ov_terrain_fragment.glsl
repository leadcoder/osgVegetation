#version 120
uniform sampler2D ov_color_texture;
varying vec3 ov_normal;
varying float ov_depth;

vec3 ov_directionalLightShadow(vec3 normal);
vec3 ov_applyFog(vec3 color, float depth);
vec4 ov_detailTexturing(vec4 base_color, float depth);

void main(void) 
{
	vec4 out_color = texture2D(ov_color_texture, gl_TexCoord[0].xy);
	out_color = ov_detailTexturing(out_color, ov_depth);

	//apply lighting and fog
	out_color.xyz *= ov_directionalLightShadow(normalize(ov_normal));
	out_color.xyz = ov_applyFog(out_color.xyz, ov_depth);
	gl_FragColor = out_color;	
}