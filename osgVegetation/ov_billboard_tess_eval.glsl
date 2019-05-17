#version 400
layout(triangles, equal_spacing, ccw) in;

in ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_in[];

out ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_out;

out vec2 ov_te_texcoord;
out vec3 ov_te_normal;
uniform mat4 osg_ModelViewProjectionMatrix;

//vec4 ov_applyTerrainElevation(vec4 pos, vec2 tex_coords);
//vec3 ov_applyTerrainNormal(vec3 normal, vec2 tex_coords);

void main()
{
	ov_out.Position = (gl_TessCoord.x * ov_in[0].Position) +
               (gl_TessCoord.y * ov_in[1].Position) +
               (gl_TessCoord.z * ov_in[2].Position);

	ov_out.TexCoord0 = (gl_TessCoord.x * ov_in[0].TexCoord0) +
                  (gl_TessCoord.y * ov_in[1].TexCoord0) +
                  (gl_TessCoord.z * ov_in[2].TexCoord0);
	
	//ov_out.Position = ov_applyTerrainElevation(pos,ov_out.TexCoord0);
	
	//vec3 a = ( ov_in[1].Position - ov_in[0].Position).xyz;
    //vec3 b = ( ov_in[2].Position - ov_in[0].Position).xyz;
	//ov_out.Normal = normalize( cross( a,b ) );
	
	ov_out.Normal  = (gl_TessCoord.x * ov_in[0].Normal) +
                     (gl_TessCoord.y * ov_in[1].Normal) +
                     (gl_TessCoord.z * ov_in[2].Normal);
	//ov_out.Normal = ov_applyTerrainNormal(ov_out.Normal, ov_out.TexCoord0);

	
//#ifdef OV_TERRAIN_ELEVATION_TEXTURE
//	pos.z = texture2D(ov_elevation_texture, tex_coord_0.xy).x;
//#endif
	//gl_Position = pos;
}