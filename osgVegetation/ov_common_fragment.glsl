#pragma import_defines (SM_LISPSM, SM_VDSM1, SM_VDSM2, FM_LINEAR, FM_EXP, FM_EXP2)

#ifdef SM_LISPSM
uniform sampler2DShadow shadowTexture;
uniform int shadowTextureUnit;
#endif

#ifdef SM_VDSM1
uniform sampler2DShadow shadowTexture0;
uniform int shadowTextureUnit0;
#endif

#ifdef SM_VDSM2
uniform sampler2DShadow shadowTexture0;
uniform int shadowTextureUnit0;
uniform sampler2DShadow shadowTexture1;
uniform int shadowTextureUnit1;
#endif

float ov_getShadow()
{
	float shadow = 1.0;
#ifdef SM_LISPSM
	shadow *= shadow2DProj(shadowTexture, gl_TexCoord[shadowTextureUnit]).r;
#endif

#ifdef SM_VDSM1
	shadow *= shadow2DProj(shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r;
#endif

#ifdef SM_VDSM2
	shadow *= shadow2DProj(shadowTexture0, gl_TexCoord[shadowTextureUnit0]).r;
	shadow *= shadow2DProj(shadowTexture1, gl_TexCoord[shadowTextureUnit1]).r;
#endif
	return shadow;
}

vec3 ov_directionalLightShadow(vec3 normal, vec3 diffuse)
{
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	//float NdotL = dot(normal, light_dir);
	float	NdotL = max(dot(normal, light_dir), 0.0);

	NdotL *= ov_getShadow();
	vec3 light = min(NdotL * diffuse*gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz, 1.0);
    
	//float NdotHV = max(dot(normal, gl_LightSource[0].halfVector.xyz), 0.0);
	//if ( NdotL * NdotHV > 0.0 )
	//	light += gl_LightSource[0].specular.xyz * pow( NdotHV, 1.0);
    //light += gl_FrontLightProduct[0].specular.xyz * pow( NdotHV, gl_FrontMaterial.shininess );
	return light;
}

vec3 ov_directionalLight(vec3 normal, vec3 diffuse)
{
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	float NdotL = max(dot(normal, light_dir), 0.0);
	vec3 light = min(NdotL * diffuse*gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz , 1.0);
	return light;
}

vec3 ov_applyFog(vec3 color, float depth)
{
#if defined(FM_LINEAR) || defined(FM_EXP) || defined(FM_EXP2)

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
	color.xyz = mix(gl_Fog.color.xyz, color.xyz, fog_factor);
#endif
	return color;
}