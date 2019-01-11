#version 420 compatibility
#extension GL_EXT_texture_array : enable 
#extension GL_EXT_gpu_shader4 : enable

in vec3 ov_position;
in vec3 ov_normal;
in vec2 ov_texCoord;
flat in float ov_textureIndex;
in vec4 ov_color;
flat in float ov_fade;    
uniform sampler2DArray ov_color_texture;
    
void main()
{
	// simple fragment shader that calculates ambient + diffuse lighting with osg::LightSource
    vec3 light_dir = normalize(vec3(gl_LightSource[0].position));
    vec3 normal   = normalize(ov_normal);
    float NdotL   = max(dot(normal, light_dir), 0.0);
	vec4 base_color = ov_color * texture2DArray(ov_color_texture, vec3(ov_texCoord.xy, ov_textureIndex));
	base_color.a *= ov_fade;
    gl_FragColor =  NdotL * base_color * gl_LightSource[0].diffuse +  base_color * gl_LightSource[0].ambient;
}