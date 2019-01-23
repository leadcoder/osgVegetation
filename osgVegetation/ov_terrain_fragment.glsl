#version 120
#pragma import_defines (SM_LISPSM, SM_VDSM1, SM_VDSM2, FM_LINEAR, FM_EXP, FM_EXP2)
uniform sampler2D ov_color_texture;
varying vec3 ov_te_normal;
varying float ov_depth;
void main(void) 
{
	vec4 out_color = texture2D( ov_color_texture, gl_TexCoord[0].xy);
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	vec3 normal = normalize(ov_te_normal);
	float NdotL = max(dot(normal, light_dir), 0);
	out_color.xyz *= min(NdotL * gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz, 2.0);

#if defined(FM_LINEAR) || defined(FM_EXP) || defined(FM_EXP2)
	float depth = ov_depth;//gl_FragCoord.z / gl_FragCoord.w;

#ifdef FM_LINEAR
	float fog_factor = (gl_Fog.end - depth) * gl_Fog.scale;
#endif

#ifdef FM_EXP
	float fog_factor = exp(-gl_Fog.density * depth);
#endif

#ifdef FM_EXP2
	float fog_factor = exp(-pow((gl_Fog.density * depth), 2.0));
#endif
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	out_color.xyz = mix(gl_Fog.color.xyz, out_color.xyz, fog_factor);
#endif

	gl_FragColor = out_color;
}