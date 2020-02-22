#pragma import_defines (OSG_LIGHTING, OSG_NUM_SHADOW_MAPS, OSG_FOG_MODE)

#ifdef OSG_NUM_SHADOW_MAPS
//pick sampler and units names from vdsm, 
//if you use other shadow-tech add own uniform or change names below
#if (OSG_NUM_SHADOW_MAPS > 0)
uniform sampler2DShadow shadowTexture0;
uniform int shadowTextureUnit0;
#if (OSG_NUM_SHADOW_MAPS > 1)
uniform sampler2DShadow shadowTexture1;
uniform int shadowTextureUnit1;
#endif
#endif
#endif

float ov_getShadow()
{
	float shadow = 1.0;
#ifdef OSG_NUM_SHADOW_MAPS
#if (OSG_NUM_SHADOW_MAPS > 0)
	shadow *= shadow2DProj(shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r;
#if (OSG_NUM_SHADOW_MAPS > 1)
	shadow *= shadow2DProj(shadowTexture1, gl_TexCoord[shadowTextureUnit1]).r;
#endif
#endif
#endif
	return shadow;
}

vec3 ov_directionalLightShadow(vec3 normal, vec3 diffuse)
{
#ifdef OSG_LIGHTING
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	float NdotL = max(dot(normal, light_dir), 0.0);

	NdotL *= ov_getShadow();
	vec3 light = min(NdotL * diffuse*gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz, 1.0);
    
	//float NdotHV = max(dot(normal, gl_LightSource[0].halfVector.xyz), 0.0);
	//if ( NdotL * NdotHV > 0.0 )
	//	light += gl_LightSource[0].specular.xyz * pow( NdotHV, 1.0);
    //light += gl_FrontLightProduct[0].specular.xyz * pow( NdotHV, gl_FrontMaterial.shininess );
	return light;
#else
	return diffuse;
#endif
}

vec3 ov_directionalLight(vec3 normal, vec3 diffuse)
{
#ifdef OSG_LIGHTING
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	float NdotL = max(dot(normal, light_dir), 0.0);
	vec3 light = min(NdotL * diffuse*gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz , 1.0);
	return light;
#else
	return diffuse;
#endif
}

vec3 ov_applyFog(vec3 color, float depth)
{
#ifdef OSG_FOG_MODE
	#if OSG_FOG_MODE == 1 //LINEAR
		float fog_factor = (gl_Fog.end - depth) * gl_Fog.scale;
	#elif OSG_FOG_MODE == 2 //EXP
		float fog_factor = exp(-gl_Fog.density * depth);
	#elif OSG_FOG_MODE == 3 //EXP2
		float fog_factor = exp(-pow((gl_Fog.density * depth), 2.0));
	#endif
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	color.xyz = mix(gl_Fog.color.xyz, color.xyz, fog_factor);
#endif
	return color;
}