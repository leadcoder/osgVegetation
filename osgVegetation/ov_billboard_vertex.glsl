//#version 140
varying vec4 ov_vertex_position;
varying vec2 ov_vertex_texcoord;
varying vec3 ov_vertex_normal;
uniform mat4 osg_ModelViewProjectionMatrix; 

void main(){
   ov_vertex_texcoord = gl_MultiTexCoord0.xy;
   ov_vertex_position = gl_Vertex;
   ov_vertex_normal = gl_Normal;//normalize(gl_NormalMatrix * gl_Normal);
   gl_Position = gl_Vertex;
   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
}