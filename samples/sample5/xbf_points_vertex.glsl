//#version 330 compatibility
//out vec4 position;
varying vec4 vPosition;
varying vec2 vTexCoord;
void main(void) 
{ 
	vTexCoord = gl_MultiTexCoord0.xy;
    gl_Position = gl_Vertex;
    vPosition = gl_Vertex;
} 