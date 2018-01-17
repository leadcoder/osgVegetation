#version 330 compatibility 
#extension GL_ARB_gpu_shader5 : enable 
uniform sampler2D ov_color_texture;
uniform float vegFadeDistance;
uniform float vegStartDistance;
uniform float vegEndDistance;
in vec2 ov_tex_coord0;
in vec3 ov_normal;
in float depth;
void main(void) { 
	vec4 finalColor = texture2D(ov_color_texture, ov_tex_coord0);
	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);
	vec3 n = normalize(ov_normal);
	float NdotL = max(dot(n, lightDir), 0);
	finalColor.xyz *= (NdotL * gl_LightSource[0].diffuse.xyz*gl_FrontMaterial.diffuse.xyz + gl_LightSource[0].ambient.xyz*gl_FrontMaterial.ambient.xyz);
	//float depth = 1.0 / gl_FragCoord.w;
	finalColor.a = mix(finalColor.a, 0, clamp((depth - 110)/20.0,0,1));
	gl_FragColor = finalColor;
}