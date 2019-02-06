#pragma import_defines (SM_LISPSM, SM_VDSM1, SM_VDSM2)

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

void ov_setShadowTexCoords(vec4 mv_pos)
{
#ifdef HAS_SHADOW
#ifdef SM_LISPSM
	int shadowTextureUnit0 = shadowTextureUnit;
#endif
	//generate coords for shadow mapping                              
	gl_TexCoord[shadowTextureUnit0].s = dot(mv_pos, gl_EyePlaneS[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].t = dot(mv_pos, gl_EyePlaneT[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].p = dot(mv_pos, gl_EyePlaneR[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].q = dot(mv_pos, gl_EyePlaneQ[shadowTextureUnit0]);
#ifdef SM_VDSM2
	gl_TexCoord[shadowTextureUnit1].s = dot(mv_pos, gl_EyePlaneS[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].t = dot(mv_pos, gl_EyePlaneT[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].p = dot(mv_pos, gl_EyePlaneR[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].q = dot(mv_pos, gl_EyePlaneQ[shadowTextureUnit1]);
#endif
#endif	
}