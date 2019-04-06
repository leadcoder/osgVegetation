#pragma import_defines (OV_TERRAIN_ELEVATION_TEXTURE)

varying vec4 ov_vertex_position;
varying vec2 ov_vertex_texcoord;
varying vec3 ov_vertex_normal;
uniform mat4 osg_ModelViewProjectionMatrix;

#ifdef OV_TERRAIN_ELEVATION_TEXTURE
uniform sampler2D ov_elevation_texture;
#endif

void main(){
   ov_vertex_texcoord = gl_MultiTexCoord0.xy;
   	vec4 pos = gl_Vertex;
#ifdef OV_TERRAIN_ELEVATION_TEXTURE
	pos.z = texture2D(ov_elevation_texture, gl_MultiTexCoord0.xy).x;
#endif
   ov_vertex_position = pos;
   ov_vertex_normal = gl_Normal;
   gl_Position = pos;
   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
}