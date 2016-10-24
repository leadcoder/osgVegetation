#version 120
#pragma import_defines ( SM_LISPSM,SM_VDSM1,SM_VDSM2,FM_LINEAR,FM_EXP,FM_EXP2 )
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_texture_array : enable
#pragma osgveg
uniform sampler2DArray baseTexture; 

#ifdef SM_LISPSM
		uniform sampler2DShadow shadowTexture;
		uniform int shadowTextureUnit;
#endif

#if SM_VDSM1
	uniform sampler2DShadow shadowTexture0;
	uniform int shadowTextureUnit0;
#endif

#ifdef SM_VDSM2
	uniform sampler2DShadow shadowTexture0;
	uniform int shadowTextureUnit0;
	uniform sampler2DShadow shadowTexture1;
	uniform int shadowTextureUnit1;
#endif		

uniform float TileRadius; 
varying vec2 TexCoord; 
varying vec3 Normal; 
varying vec3 Color; 
varying float TextureIndex; 

void main(void) 
{ 
   vec4 outColor = texture2DArray( baseTexture, vec3(TexCoord, TextureIndex)); 
   outColor.xyz *= Color; 
   float depth = gl_FragCoord.z / gl_FragCoord.w;
   vec3 lightDir = normalize(gl_LightSource[0].position.xyz);
   vec3 normal = normalize(Normal);
   //add diffuse lighting 
   float NdotL = max(dot(normal, lightDir), 0);

#ifdef SM_LISPSM
   float shadow = shadow2DProj( shadowTexture, gl_TexCoord[shadowTextureUnit] ).r;   
   NdotL *= shadow;
#endif

#if SM_VDSM1
	float shadow0 = shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r;
	NdotL *= shadow0;
#endif

#ifdef SM_VDSM2
	float shadow0 = shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r;
	float shadow1 = shadow2DProj( shadowTexture1, gl_TexCoord[shadowTextureUnit1] ).r;
	NdotL *= shadow0*shadow1; \n";
#endif
   outColor.xyz *= (NdotL * gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz);
   //outColor.w = outColor.w * clamp(1.0 - ((depth-TileRadius)/(TileRadius*0.1)), 0.0, 1.0);
#ifdef FM_LINEAR
   float fogFactor = (gl_Fog.end - depth) * gl_Fog.scale;
#endif

#ifdef FM_EXP
	float fogFactor = exp(-gl_Fog.density * depth);
#endif

#ifdef FM_EXP2
   float fogFactor = exp(-pow((gl_Fog.density * depth), 2.0));
#endif

#if defined(FM_LINEAR) || defined(FM_EXP) || defined(FM_EXP2)
	fogFactor = clamp(fogFactor, 0.0, 1.0);
    outColor.xyz = mix(gl_Fog.color.xyz, outColor.xyz, fogFactor);
#endif				
   gl_FragColor = outColor;
}
