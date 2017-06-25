#version 120
#extension GL_ARB_geometry_shader4 : enable

in vec2 texcoord[];
varying vec2 otexcoord;
varying vec4 color;
flat varying int veg_type;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform sampler2D baseTexture;
uniform int numBillboards;
uniform vec4 billboardData[10];
uniform float vegMaxDistance;

float veg_rangeRand(float minValue, float maxValue, vec2 co)
{
    float t = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
    return minValue + t*(maxValue-minValue);
}

float veg_rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Generate a pseudo-random barycentric point inside a triangle.
vec3 veg_getRandomBarycentricPoint(vec2 seed)
{
    vec3 b;
    b[0] = veg_rand(seed.xy);
    b[1] = veg_rand(seed.yx);
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
	vec4 pos = vec4(0,0,0,1);
    vec2 texc = vec2(0,0);
    
	//gen a random point in triangle
    vec3 b = veg_getRandomBarycentricPoint(gl_PositionIn[0].xy);
    for(int i=0; i < 3; ++i)
    {
        pos.x += b[i] * gl_PositionIn[i].x;
        pos.y += b[i] * gl_PositionIn[i].y;
        pos.z += b[i] * gl_PositionIn[i].z;
        
        texc.x += b[i] * texcoord[i].x;
        texc.y += b[i] * texcoord[i].y;
    }
	
	
	color = texture2D( baseTexture, texc);
	
	//we dont have any landcover data, just use color map...
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
	vec4 mv_pos = gl_ModelViewMatrix * pos;

	float fadeDist = vegMaxDistance/3.0;
	float fadeStartDist =  vegMaxDistance - fadeDist;
	float scale = 1 - clamp((-mv_pos.z-fadeStartDist) / fadeDist, 0.0, 1.0);

	//culling
    if ( scale == 0.0 )
        return;
	
	float rand_scale = veg_rangeRand(0.5, 1.5, pos.xy /* * 50*/);
	scale = scale*scale*scale*rand_scale;
	
	veg_type = int(floor(float(numBillboards)*veg_rangeRand(0, 1.0, pos.xy)));
	veg_type = min(veg_type, numBillboards - 1);
	vec4 billboard = billboardData[veg_type];
	vec3 left = vec3(scale*billboard.x, 0, 0);
	
	vec4 up_vec = gl_ModelViewMatrix*vec4(0.0, 0,scale*billboard.y,0);
	
	vec3 up = up_vec.xyz;//vec3(0.0, scale*billboard.y,0.0);//Up direction in OSG
	//vec3 up = vec3(0.0, 0.0, scale*1.0);//Up direction in OSG
	//vec3 left = vec3(-dir.y, dir.x, 0);
	//left = normalize(left);
	//left.x *= 0.5*scale;
	//left.y *= 0.5*scale;
	e.xyz = mv_pos.xyz + left;   gl_Position = osg_ProjectionMatrix* e; otexcoord = vec2(0.0, 0.0);  EmitVertex();
	e.xyz = mv_pos.xyz - left;   gl_Position = osg_ProjectionMatrix * e; otexcoord = vec2(1.0, 0.0);  EmitVertex();
	e.xyz = mv_pos.xyz + left + up;  gl_Position = osg_ProjectionMatrix * e; otexcoord = vec2(0.0, 1.0);  EmitVertex();
	e.xyz = mv_pos.xyz - left + up;  gl_Position = osg_ProjectionMatrix * e; otexcoord = vec2(1.0, 1.0);  EmitVertex();
	EndPrimitive();
}