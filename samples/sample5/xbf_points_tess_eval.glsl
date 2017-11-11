#version 400
layout(triangles, equal_spacing, ccw) in;

in vec4 tcPosition[];
in vec2 tcTexCoord[];
out vec4 tePosition;
out vec2 teTexCoord;
//in float distScale[];

void main(){
	
	tePosition = ((gl_TessCoord.x * tcPosition[0]) +
                  (gl_TessCoord.y * tcPosition[1]) +
                  (gl_TessCoord.z * tcPosition[2]));
				  
	teTexCoord  = (gl_TessCoord.x * tcTexCoord[0]) +
                 (gl_TessCoord.y * tcTexCoord[1]) +
                 (gl_TessCoord.z * tcTexCoord[2]);
}