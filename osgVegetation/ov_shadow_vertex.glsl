#version 400 compatibility
#pragma import_defines (OSG_NUM_SHADOW_MAPS, OE_SHADOW_NUM_SLICES)

#ifdef OSG_NUM_SHADOW_MAPS
uniform int osg_ShadowTextureUnit0;
uniform int osg_ShadowTextureUnit1;

void ov_setShadowTexCoords(vec4 mv_pos)
{
#if (OSG_NUM_SHADOW_MAPS > 0)
	//generate coords for shadow mapping
	gl_TexCoord[osg_ShadowTextureUnit0].s = dot(mv_pos, gl_EyePlaneS[osg_ShadowTextureUnit0]);
	gl_TexCoord[osg_ShadowTextureUnit0].t = dot(mv_pos, gl_EyePlaneT[osg_ShadowTextureUnit0]);
	gl_TexCoord[osg_ShadowTextureUnit0].p = dot(mv_pos, gl_EyePlaneR[osg_ShadowTextureUnit0]);
	gl_TexCoord[osg_ShadowTextureUnit0].q = dot(mv_pos, gl_EyePlaneQ[osg_ShadowTextureUnit0]);
#if (OSG_NUM_SHADOW_MAPS > 1)
	gl_TexCoord[osg_ShadowTextureUnit1].s = dot(mv_pos, gl_EyePlaneS[osg_ShadowTextureUnit1]);
	gl_TexCoord[osg_ShadowTextureUnit1].t = dot(mv_pos, gl_EyePlaneT[osg_ShadowTextureUnit1]);
	gl_TexCoord[osg_ShadowTextureUnit1].p = dot(mv_pos, gl_EyePlaneR[osg_ShadowTextureUnit1]);
	gl_TexCoord[osg_ShadowTextureUnit1].q = dot(mv_pos, gl_EyePlaneQ[osg_ShadowTextureUnit1]);
#endif
#endif
}

#elif defined(OE_SHADOW_NUM_SLICES)

uniform mat4 oe_shadow_matrix[OE_SHADOW_NUM_SLICES];
out vec4 oe_shadow_coord[OE_SHADOW_NUM_SLICES];

void ov_setShadowTexCoords(vec4 mv_pos)
{
	for(int i=0; i < OE_SHADOW_NUM_SLICES; ++i)
    {
        oe_shadow_coord[i] = oe_shadow_matrix[i] * mv_pos;
    }
}
#else
void ov_setShadowTexCoords(vec4 mv_pos)
{
}

#endif