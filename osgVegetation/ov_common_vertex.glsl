#pragma import_defines (OSG_NUM_SHADOW_MAPS)

uniform int shadowTextureUnit0;
uniform int shadowTextureUnit1;


void ov_setShadowTexCoords(vec4 mv_pos)
{
#ifdef OSG_NUM_SHADOW_MAPS
#if (OSG_NUM_SHADOW_MAPS > 0)
	//generate coords for shadow mapping
	gl_TexCoord[shadowTextureUnit0].s = dot(mv_pos, gl_EyePlaneS[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].t = dot(mv_pos, gl_EyePlaneT[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].p = dot(mv_pos, gl_EyePlaneR[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].q = dot(mv_pos, gl_EyePlaneQ[shadowTextureUnit0]);
#if (OSG_NUM_SHADOW_MAPS > 1)
	gl_TexCoord[shadowTextureUnit1].s = dot(mv_pos, gl_EyePlaneS[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].t = dot(mv_pos, gl_EyePlaneT[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].p = dot(mv_pos, gl_EyePlaneR[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].q = dot(mv_pos, gl_EyePlaneQ[shadowTextureUnit1]);
#endif
#endif
#endif	
}