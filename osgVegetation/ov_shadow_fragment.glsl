#version 400 compatibility
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
#pragma import_defines (OSG_NUM_SHADOW_MAPS, OE_SHADOW_NUM_SLICES, OE_IS_DEPTH_CAMERA)

#ifdef OSG_NUM_SHADOW_MAPS
//pick sampler and units names from vdsm, 
//if you use other shadow-tech add own uniform or change names below

#if (OSG_NUM_SHADOW_MAPS > 0)
uniform sampler2DShadow shadowTexture0;
uniform int shadowTextureUnit0;
#endif

#if (OSG_NUM_SHADOW_MAPS > 1)
uniform sampler2DShadow shadowTexture1;
uniform int shadowTextureUnit1;
#endif

uniform float osg_shadowMaxDistance;
//uniform float osg_shadowFadeOutDistance;
uniform float osg_shadowSoftness;

float ov_getPCFShadowMapValue(sampler2DShadow shadowmap, vec4 shadowUV)
{
	// PCF filtering
	ivec2 tex_size = textureSize(shadowmap,0);
	float invTexel = 1.0 / float(tex_size.x);
	float offset  = osg_shadowSoftness * invTexel * shadowUV.w;
	float shadowTerm = shadow2DProj(shadowmap, shadowUV).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV - vec4(offset, 0.0, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV + vec4(offset, 0.0, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV - vec4(0.0, offset, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV + vec4(0.0, offset, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV - vec4(offset, offset, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV + vec4(offset, offset, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV - vec4(offset,-offset, 0.0, 0.0)).r;
	shadowTerm += shadow2DProj(shadowmap, shadowUV + vec4(offset,-offset, 0.0, 0.0)).r;
	shadowTerm = shadowTerm / 9.0;
	return shadowTerm;
}

float ov_getShadowMapValue(sampler2DShadow shadowmap, vec4 shadowUV, float bias)
{
	shadowUV.z -= bias;
	if(osg_shadowSoftness > 0)
		return ov_getPCFShadowMapValue(shadowmap, shadowUV);
	else
		return shadow2DProj(shadowmap, shadowUV).r;
}

float ov_getShadow(vec3 normal, float depth)
{
	const float b0 = 0.001;
    vec3 L = normalize(gl_LightSource[0].position.xyz);
    vec3 N = normalize(normal);
    float costheta = clamp(dot(L,N), 0.0, 1.0);
    float bias = b0*tan(acos(costheta));

	float shadow = 1.0;
#ifdef OSG_NUM_SHADOW_MAPS
#if (OSG_NUM_SHADOW_MAPS > 0)
	shadow *= ov_getShadowMapValue(shadowTexture0, gl_TexCoord[shadowTextureUnit0], bias);
#if (OSG_NUM_SHADOW_MAPS > 1)
	shadow *= ov_getShadowMapValue(shadowTexture1, gl_TexCoord[shadowTextureUnit1], bias);
#endif
#endif
#endif
	//float depth = gl_FragCoord.z / gl_FragCoord.w;
	float fade_dist = osg_shadowMaxDistance*0.2;
	float intp = min( max(osg_shadowMaxDistance - depth, 0) / fade_dist, 1.0);
	return mix(1.0, shadow,intp);
}

#elif defined(OE_SHADOW_NUM_SLICES)

in vec4 oe_shadow_coord[OE_SHADOW_NUM_SLICES];
uniform sampler2DArrayShadow oe_shadow_map;
uniform float oe_shadow_blur;

float ov_getShadowValue(sampler2DArrayShadow shadowmap, vec4 shadowUV)
{
	if(oe_shadow_blur > 0)
	{
		// PCF filtering
		ivec3 tex_size = textureSize(shadowmap,0);
		float invTexel = 1.0 / float(tex_size.x);
		float offset  = oe_shadow_blur * invTexel / shadowUV.w;
		float shadowTerm = shadow2DArray(shadowmap, shadowUV).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV - vec4(offset, 0.0, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV + vec4(offset, 0.0, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV - vec4(0.0, offset, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV + vec4(0.0, offset, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV - vec4(offset, offset, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV + vec4(offset, offset, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV - vec4(offset,-offset, 0.0, 0.0)).r;
		shadowTerm += shadow2DArray(shadowmap, shadowUV + vec4(offset,-offset, 0.0, 0.0)).r;
		shadowTerm = shadowTerm / 9.0;
		return shadowTerm;
	}
	else
		return shadow2DArray(shadowmap, shadowUV).r;
}

float ov_getShadow(vec3 normal,float depth)
{
    float factor = 1.0;
    // pre-pixel biasing to reduce moire/acne
    const float b0 = 0.001;
    vec3 L = normalize(gl_LightSource[0].position.xyz);
    vec3 N = normalize(normal);
    float costheta = clamp(dot(L,N), 0.0, 1.0);
    float bias = b0*tan(acos(costheta));
    // loop over the slices:
    for(int i=0; i< OE_SHADOW_NUM_SLICES && factor > 0.0; ++i)
    {
        vec4 c = oe_shadow_coord[i];
        vec3 coord = vec3(c.x, c.y, float(i));
        factor = min(factor,ov_getShadowValue(oe_shadow_map, vec4(coord,c.z - bias)));
    }
    return factor;
}

#else
float ov_getShadow(vec3 normal, float depth)
{
	return 1.0;
}
#endif
