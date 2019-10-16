#version 420 compatibility
#extension GL_EXT_texture_array : enable 
#extension GL_EXT_gpu_shader4 : enable

in vec3 ov_position;
in vec3 ov_vPosition;
in vec3 ov_normal;
in vec2 ov_texCoord;
in vec3 ov_diffuse;
flat in mat3 ov_texMat;
flat in float ov_textureIndex;
in vec4 ov_color;
flat in float ov_fade;
flat in int ov_type;
uniform sampler2DArray ov_mesh_color_texture;
    
//forward declarations
vec3 ov_directionalLight(vec3 normal, vec3 diffuse);
vec3 ov_directionalLightShadow(vec3 normal, vec3 diffuse);
vec3 ov_applyFog(vec3 color, float depth);

void main()
{
	// simple fragment shader that calculates ambient + diffuse lighting with osg::LightSource
    //vec3 light_dir = normalize(vec3(gl_LightSource[0].position));
    //vec3 normal   = normalize(ov_normal);
    //float NdotL   = max(dot(normal, light_dir), 0.0);
	vec4 base_color = ov_color * texture2DArray(ov_mesh_color_texture, vec3(ov_texCoord.xy, ov_textureIndex));
	vec3 normal = normalize(ov_texMat * normalize(ov_normal));
	if(ov_type > 0)
	{
		vec3 coded_normal = normalize(texture2DArray(ov_mesh_color_texture, vec3(ov_texCoord.xy - vec2(0.0,0.5), ov_textureIndex)).xyz);
		vec3 bnormal = normalize(ov_texMat * ((coded_normal * 2.0) - 1));
		//vec3 bnormal = normalize(vec3(ov_vPosition.xy,0));
		//bnormal = normalize(ov_texMat * vec3(bnormal.xy,ov_texCoord.y));
		base_color.xyz *= ov_directionalLight(bnormal, ov_diffuse);
		normal.y = 0;
		normal = normalize(normal);
		float NdotC = dot(normal, vec3(0,0,1));
		if(NdotC < 0)
			base_color.a = 0;
		else
			base_color.a *= NdotC*NdotC*NdotC*NdotC*NdotC*NdotC;
	}
	else
	{
		//if (!gl_FrontFacing)
		//	normal = -normal;
	
		//base_color.xyz *= ov_directionalLight(normal);
		base_color.xyz *= ov_directionalLightShadow(normal, ov_diffuse);
	}

	base_color.xyz = ov_applyFog(base_color.xyz, -ov_position.z);

	float fade_dist = 100;
	float alpha_boost = 1.0 + 1.0 * clamp(-ov_position.z / fade_dist, 0.0, 1.0);
	base_color.a *= ov_fade*alpha_boost;

	gl_FragColor =  base_color;
    //gl_FragColor =  NdotL * base_color * gl_LightSource[0].diffuse +  base_color * gl_LightSource[0].ambient;
}