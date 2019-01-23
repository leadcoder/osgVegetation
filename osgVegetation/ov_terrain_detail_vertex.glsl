//#version 140
#pragma import_defines (SM_LISPSM, SM_VDSM1, SM_VDSM2)
uniform mat4 osg_ModelViewProjectionMatrix;
varying vec3 ov_normal;
varying vec2 ov_tex_coord0;
varying float ov_depth;

#if defined(SM_LISPSM) || defined(SM_VDSM1) || defined(SM_VDSM2)
#define HAS_SHADOW
#endif

#ifdef SM_LISPSM
uniform int shadowTextureUnit;
#endif

#ifdef SM_VDSM1
uniform int shadowTextureUnit0;
#endif

#ifdef SM_VDSM2
uniform int shadowTextureUnit0;
uniform int shadowTextureUnit1;
#endif

void ov_shadow(vec4 ecPosition)
{
#ifdef HAS_SHADOW
#ifdef SM_LISPSM
	int shadowTextureUnit0 = shadowTextureUnit;
#endif
	ecPosition = gl_ModelViewMatrix * ecPosition;
	// generate coords for shadow mapping                              
	gl_TexCoord[shadowTextureUnit0].s = dot(ecPosition, gl_EyePlaneS[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].t = dot(ecPosition, gl_EyePlaneT[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].p = dot(ecPosition, gl_EyePlaneR[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].q = dot(ecPosition, gl_EyePlaneQ[shadowTextureUnit0]);
#ifdef SM_VDSM2
	gl_TexCoord[shadowTextureUnit1].s = dot(ecPosition, gl_EyePlaneS[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].t = dot(ecPosition, gl_EyePlaneT[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].p = dot(ecPosition, gl_EyePlaneR[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].q = dot(ecPosition, gl_EyePlaneQ[shadowTextureUnit1]);
#endif
#endif	
}

void main() {
	gl_Position = osg_ModelViewProjectionMatrix* gl_Vertex;
	ov_shadow(gl_Vertex);
	ov_normal = normalize(gl_NormalMatrix * gl_Normal);
	ov_tex_coord0 = gl_MultiTexCoord0.xy;
	vec4 mvm_pos = gl_ModelViewMatrix * gl_Vertex;
	ov_depth = length(mvm_pos.xyz);
}