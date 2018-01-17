#version 330 compatibility 
#extension GL_ARB_gpu_shader5 : enable 
uniform sampler2D ov_color_texture;
uniform float ov_FadeDistance;
uniform float ov_StartDistance;
uniform float ov_EndDistance;
in vec2 ov_tex_coord0;
in vec3 ov_normal;
in float ov_depth;
void main(void) { 
	vec4 finalColor = texture2D(ov_color_texture, ov_tex_coord0);
	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);
	vec3 n = normalize(ov_normal);
	float NdotL = max(dot(n, lightDir), 0);
	finalColor.xyz *= (NdotL * gl_LightSource[0].diffuse.xyz*gl_FrontMaterial.diffuse.xyz + gl_LightSource[0].ambient.xyz*gl_FrontMaterial.ambient.xyz);
	finalColor.a = mix(finalColor.a, 0, clamp((ov_depth - (ov_EndDistance + ov_FadeDistance*0.5))/ov_FadeDistance, 0, 1));
	gl_FragColor = finalColor;
}