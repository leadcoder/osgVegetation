//#version 140
//in vec4 osg_Vertex;
//	in vec4 osg_MultiTexCoord0;
varying vec4 vPosition;
varying vec2 vTexcoord;
//uniform sampler2D terrainTexture;
uniform vec3 terrainOrigin;
uniform vec3 terrainScaleDown;
//uniform mat4 osg_ProjectionMatrix;
//uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix; 

void main(){
	vTexcoord = gl_MultiTexCoord0.xy;
	vec4 position;
	position.x = gl_Vertex.x;
	position.y = gl_Vertex.y;
	position.z = gl_Vertex.z;
	position.w = gl_Vertex.w;
   gl_Position     = position;
   vPosition = position;
   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
}