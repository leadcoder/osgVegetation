#version 330 compatibility 
#extension GL_ARB_gpu_shader5 : enable 
uniform sampler2D baseTexture; 
in vec2 texcoord; 
in vec3 normal; 
void main(void) { 
	vec4 finalColor = texture2D( baseTexture, texcoord); 
	vec3 lightDir = normalize(gl_LightSource[0].position.xyz);
	vec3 n = normalize(normal);
	float NdotL = max(dot(n, lightDir), 0);
	finalColor.xyz *= (NdotL * gl_LightSource[0].diffuse.xyz*gl_FrontMaterial.diffuse.xyz + gl_LightSource[0].ambient.xyz*gl_FrontMaterial.ambient.xyz);
	gl_FragColor = finalColor;
} ;