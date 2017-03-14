#version 120
#extension GL_ARB_geometry_shader4 : enable

in vec2 texcoord[];
varying vec2 otexcoord;
varying vec4 color;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform sampler2D baseTexture;


float oe_landcover_rangeRand(float minValue, float maxValue, vec2 co)
{
    float t = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
    return minValue + t*(maxValue-minValue);
}


// Generate a pseudo-random barycentric point inside a triangle.
vec3 oe_landcover_getRandomBarycentricPoint(vec2 seed)
{
    vec3 b;
    b[0] = oe_landcover_rangeRand(0.0, 1.0, seed.xy);
    b[1] = oe_landcover_rangeRand(0.0, 1.0, seed.yx);
    if (b[0]+b[1] >= 1.0)
    {
        b[0] = 1.0 - b[0];
        b[1] = 1.0 - b[1];
    }
    b[2] = 1.0 - b[0] - b[1];
    return b;
}


void main(void)
{
	//vec4 pos = gl_PositionIn[0] + gl_PositionIn[1] + gl_PositionIn[2];
	//pos *= 1.0 / 3.0;
	//vec2 texc = texcoord[0] + texcoord[1] + texcoord[2];
	//texc *= 1.0 / 3.0;
	
	vec4 pos = vec4(0,0,0,1);
    vec2 texc = vec2(0,0);
    
	// gen a random point within the input triangle
    vec3 b = oe_landcover_getRandomBarycentricPoint(gl_PositionIn[0].xy);
    
    // Load the triangle data and compute the new position and tile coords
    // using the barycentric coordinates.
    for(int i=0; i < 3; ++i)
    {
        pos.x += b[i] * gl_PositionIn[i].x;
        pos.y += b[i] * gl_PositionIn[i].y;
        pos.z += b[i] * gl_PositionIn[i].z;
        
        texc.x += b[i] * texcoord[i].x;
        texc.y += b[i] * texcoord[i].y;
    }
	
	
	color = texture2D( baseTexture, texc);
	if(color.y > 0.4)
		return;
	color *= 2.2;
	vec4 camera_pos = gl_ModelViewMatrixInverse[3];
	vec4 e;
	e.w = pos.w;
	vec3 dir = camera_pos.xyz - pos.xyz;
	//we are only interested in xy-plane direction
	dir.z = 0;
	dir = normalize(dir);
	vec4 center_view = gl_ModelViewMatrix * pos;
	float scale = 1 - clamp((-center_view.z-50) / 100, 0.0, 1.0);
	 // Distance culling:
    if ( scale == 0.0 )
        return;
	scale = scale*scale*scale;
	vec3 up = vec3(0.0, 0.0, scale*1.0);//Up direction in OSG
	vec3 left = vec3(-dir.y, dir.x, 0);
	left = normalize(left);
	left.x *= 0.5*scale;
	left.y *= 0.5*scale;
	e.xyz = pos.xyz + left;   gl_Position = osg_ModelViewProjectionMatrix* e; otexcoord = vec2(0.0, 0.0);  EmitVertex();
	e.xyz = pos.xyz - left;   gl_Position = osg_ModelViewProjectionMatrix * e; otexcoord = vec2(1.0, 0.0);  EmitVertex();
	e.xyz = pos.xyz + left + up;  gl_Position = osg_ModelViewProjectionMatrix * e; otexcoord = vec2(0.0, 1.0);  EmitVertex();
	e.xyz = pos.xyz - left + up;  gl_Position = osg_ModelViewProjectionMatrix * e; otexcoord = vec2(1.0, 1.0);  EmitVertex();
	EndPrimitive();
}
/*
for (int i = 0; i < gl_VerticesIn; i++)
{
gl_Position = osg_ModelViewProjectionMatrix*gl_PositionIn[i];
otexcoord = texcoord[i];
EmitVertex();
}
EndPrimitive();
}*/
