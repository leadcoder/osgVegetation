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
	
	/*float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	vec3 a = mix(tcPosition[1].xyz, tcPosition[0].xyz, u);
	vec3 b = mix(tcPosition[2].xyz, tcPosition[3].xyz, u);
	vec3 position = mix(a, b, v);
	vec2 a2 = mix(tcTexcoord[1], tcTexcoord[0], u);
	vec2 b2 = mix(tcTexcoord[2], tcTexcoord[3], u);
	texcoord = mix(a2, b2, v);
	gl_Position = vec4(position,1);
	*/
	
	gl_Position = (gl_TessCoord.x * tcPosition[0]) +
                  (gl_TessCoord.y * tcPosition[1]) +
                  (gl_TessCoord.z * tcPosition[2]);
				  
	texcoord  = (gl_TessCoord.x * tcTexcoord[0]) +
                  (gl_TessCoord.y * tcTexcoord[1]) +
                  (gl_TessCoord.z * tcTexcoord[2]);
	
	//position.z = texture2D(terrainTexture, texcoord).r;
	//    tePatchDistance = gl_TessCoord;
	//    tePosition = normalize(p0 + p1 + p2);
	//    gl_Position = osg_ModelViewProjectionMatrix * vec4(position,1);
	
}