#pragma import_defines (SM_LISPSM, SM_VDSM1, SM_VDSM2, FM_LINEAR, FM_EXP, FM_EXP2)

#if defined(SM_LISPSM) || defined(SM_VDSM1) || defined(SM_VDSM2)
#define HAS_SHADOW
#endif

#ifdef HAS_SHADOW

#if defined(SM_LISPSM) || defined(SM_VDSM1)
	#define OV_NUM_SHADOW_MAPS 1
#endif

#ifdef SM_VDSM2
	#define OV_NUM_SHADOW_MAPS 2
#endif
//Sampler name differ between lispsm and vdsm
#ifdef SM_LISPSM
#define OV_SHADOW_SAMPPLER0_NAME shadowTexture
#else
#define OV_SHADOW_SAMPPLER0_NAME shadowTexture0
#endif

uniform sampler2DShadow OV_SHADOW_SAMPPLER0_NAME;
uniform int shadowTextureUnit0;
uniform sampler2DShadow shadowTexture1;
uniform int shadowTextureUnit1;

#endif

float ov_getShadow()
{
	float shadow = 1.0;
#ifdef HAS_SHADOW
	shadow *= shadow2DProj(OV_SHADOW_SAMPPLER0_NAME, gl_TexCoord[shadowTextureUnit0]).r;
#if (OV_NUM_SHADOW_MAPS > 1)
	shadow *= shadow2DProj(shadowTexture1, gl_TexCoord[shadowTextureUnit1]).r;
#endif
#endif
	return shadow;
}

vec3 ov_directionalLightShadow(vec3 normal, vec3 diffuse)
{
	vec3 light_dir = normalize(gl_LightSource[0].position.xyz);
	float NdotL = max(dot(normal, light_dir), 0.0);

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