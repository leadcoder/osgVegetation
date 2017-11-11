#version 330 compatibility
#extension GL_ARB_gpu_shader5 : enable
in vec4 xfb_position;
out vec2 texcoord; 
out vec3 normal; 
void main(void) { 
	vec4 position = gl_Vertex; 
	position.xyz += xfb_position.xyz; 
	gl_Position = gl_ModelViewProjectionMatrix * (position); 
	normal = normalize(gl_NormalMatrix * gl_Normal);
	texcoord = gl_MultiTexCoord0.xy; 
} ;