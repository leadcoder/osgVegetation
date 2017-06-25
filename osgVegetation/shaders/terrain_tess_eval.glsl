#version 400
layout(triangles, equal_spacing, ccw) in;
in vec4 tcPosition[];
in vec2 tcTexcoord[];
in float distScale[];
out vec2 texcoord;
//uniform sampler2D terrainTexture;
//out vec3 tePatchDistance;
//uniform mat4 osg_ProjectionMatrix;
//uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main(){
	gl_Position = osg_ModelViewProjectionMatrix* ((gl_TessCoord.x * tcPosition[0]) +
                  (gl_TessCoord.y * tcPosition[1]) +
                  (gl_TessCoord.z * tcPosition[2]));
				  
	texcoord  = (gl_TessCoord.x * tcTexcoord[0]) +
                  (gl_TessCoord.y * tcTexcoord[1]) +
                  (gl_TessCoord.z * tcTexcoord[2]);
	
}