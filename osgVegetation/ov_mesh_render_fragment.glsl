#version 420 compatibility
#extension GL_EXT_texture_array : enable 
#extension GL_EXT_gpu_shader4 : enable
#pragma import_defines (OE_IS_DEPTH_CAMERA, OV_OVERRIDE_NORMALS)

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
	vec4 base_color = ov_color * texture2DArray(ov_mesh_color_texture, vec3(ov_texCoord.xy, ov_textureIndex));
#ifdef OE_IS_DEPTH_CAMERA
	gl_FragColor =  base_color;
#else
	vec3 normal = normalize(ov_texMat * normalize(ov_normal));
	if(ov_type > 0) // impostor
	{
		//Do impostor plane fading
		if(ov_normal.z < 0.9)
			normal.y = 0;
		normal = normalize(normal);
		float NdotC = max(dot(normal, vec3(0,0,1)),0);
		base_color.a *= NdotC*NdotC*NdotC*NdotC;

#ifdef OV_OVERRIDE_NORMALS
		normal = normalize(vec3(ov_vPosition.xy,0));
		normal = normalize(ov_texMat * normalize(normal + normalize(ov_normal)));
#else
		vec3 coded_normal = normalize(texture2DArray(ov_mesh_color_texture, vec3(ov_texCoord.xy - vec2(0.0,0.5), ov_textureIndex)).xyz);
		normal = normalize(ov_texMat * ((coded_normal * 2.0) - 1));
#endif
	}
	else //regular mesh
	{
		if (!gl_FrontFacing)
			normal = -normal;
#ifdef OV_OVERRIDE_NORMALS
		normal = normalize(vec3(ov_vPosition.xy,0));
		normal = normalize(ov_texMat * vec3(normal.xy, 0));
#endif
	}

	base_color.xyz *= ov_directionalLightShadow(normal, ov_diffuse);
	base_color.xyz = ov_applyFog(base_color.xyz, -ov_position.z);

	//boost alpha at distance to make forest more dens 
	float alpha_boost_fade_dist = 100;
	float alpha_boost = 1.0 + 1.0 * clamp(-ov_position.z / alpha_boost_fade_dist, 0.0, 1.0);
	base_color.a *= ov_fade*alpha_boost;
	gl_FragColor =  base_color;
#endif
}