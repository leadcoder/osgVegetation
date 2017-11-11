#version 400
layout(vertices = 3) out;
in vec4 vPosition[];
in vec2 vTexCoord[];
out vec4 tcPosition[];
out vec2 tcTexCoord[];
uniform mat4 osg_ModelViewMatrix;
uniform float vegMaxDistance;
uniform int vegDensity;
#define ID gl_InvocationID
void main(){
	tcPosition[ID] = vPosition[ID];
	tcTexCoord[ID] = vTexCoord[ID];
	if (ID == 0) {
		float level = 2;
		vec4 mv_pos1 = osg_ModelViewMatrix * vPosition[0];
		vec4 mv_pos2 = osg_ModelViewMatrix * vPosition[1];
		vec4 mv_pos3 = osg_ModelViewMatrix * vPosition[2];
		//float dist =  - max( max(mv_pos1.z, mv_pos2.z), mv_pos3.z);
		float dist =  min( min(length(mv_pos1.xyz), length(mv_pos2.xyz)), length(mv_pos3.xyz));
		//float vegMaxDistance = 2000;
		if(dist > 0.1 && dist < vegMaxDistance) 
		{
			level = vegDensity;   
		}
		else 
		{ 
			level = 0;
		}
		
		gl_TessLevelInner[0] = level;
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
	}
}