varying vec4 ov_vertex_position;
varying vec2 ov_vertex_texcoord;

uniform mat4 osg_ModelViewProjectionMatrix; 

void main(){
	ov_vertex_texcoord = gl_MultiTexCoord0.xy;
	vec4 position;
	position.x = gl_Vertex.x;
	position.y = gl_Vertex.y;
	position.z = gl_Vertex.z;
	position.w = gl_Vertex.w;
    gl_Position     = position;
    ov_vertex_position = position;
	gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
}