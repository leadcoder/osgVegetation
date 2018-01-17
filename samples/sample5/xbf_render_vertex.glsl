#version 330 compatibility
#extension GL_ARB_gpu_shader5 : enable
in vec4 xfb_position;
out vec2 ov_tex_coord0;
out vec3 ov_normal;
out float depth;
void main(void) { 
	vec4 position = gl_Vertex; 
	position.xyz += xfb_position.xyz; 
	gl_Position = gl_ModelViewProjectionMatrix * (position); 
	ov_normal = normalize(gl_NormalMatrix * gl_Normal);
	vec4 mvm_pos = gl_ModelViewMatrix * position;
	depth = length(mvm_pos.xyz);
	ov_tex_coord0 = gl_MultiTexCoord0.xy;
}