//#version 140
varying vec4 ov_vertex_position;
varying vec2 ov_vertex_texcoord;
uniform mat4 osg_ModelViewProjectionMatrix; 

//uniform sampler2D ov_terrain_texture;
void main(){
   ov_vertex_texcoord = gl_MultiTexCoord0.xy;
   ov_vertex_position = gl_Vertex;
   gl_Position = gl_Vertex;
   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
   //position.z = texture2D(ov_terrain_texture, gl_MultiTexCoord0.xy).r;
   //gl_Position     = osg_ModelViewProjectionMatrix * position;
}