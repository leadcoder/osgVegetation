#version 400 compatibility
#extension GL_ARB_geometry_shader4 : enable
#extension GL_ARB_enhanced_layouts : enable
layout(triangles) in; 
layout(points, max_vertices=2) out; 
uniform float vegDistanceLOD0;
uniform float vegFadeDistance; 
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform sampler2D baseTexture;
uniform sampler2D landCoverTexture;
in vec4 tePosition[3];
in vec2 teTexCoord[3]; 
layout(stream=0) out vec4 xfb_output_lod0;
layout(stream=1) out vec4 xfb_output_lod1;

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

void main(void) { 
	//vec4 vertView = gl_ModelViewMatrix * gPosition[0]; 
//    vertView.xy -= cullingRadius*sign(vertView.xy); 
	//vec4 vertClip = gl_ProjectionMatrix * vertView; 
	//float w = abs(vertClip.w);
	
	vec4 pos = vec4(0,0,0,1);
	vec2 texc = vec2(0,0);
	//gen a random point in triangle
    vec3 b = veg_getRandomBarycentricPoint(tePosition[0].xy);
    for(int i=0; i < 3; ++i)
    {
        pos.x += b[i] * tePosition[i].x;
        pos.y += b[i] * tePosition[i].y;
        pos.z += b[i] * tePosition[i].z;
    
		texc.x += b[i] * teTexCoord[i].x;
        texc.y += b[i] * teTexCoord[i].y;
    }
	
	
	vec4 baseColor = texture2D( baseTexture, texc);
	float intensity = length(baseColor.xyz);
	
	if(intensity > 0.4)
		return;
	
	vec4 lcColor = texture2D( landCoverTexture, texc);
	
	if(length(lcColor.xyz) > 0.2)
		return;

	intensity =  0.2 + 2 * intensity;
	
	vec4 mvm_pos = osg_ModelViewMatrix * pos;
	float distance = length(mvm_pos.xyz);
	//float distance = -mvm_pos.z;//length(mvm_pos);
	
	if ( distance < vegDistanceLOD0 && distance > 0 ) {
	
		xfb_output_lod0 = pos;
		xfb_output_lod0.w = intensity;
		EmitStreamVertex(0);
	}
	else if ( distance < (vegDistanceLOD0 + vegFadeDistance*4) && distance > 0 ) {

    	xfb_output_lod0 = pos;
		xfb_output_lod0.w = intensity;
		EmitStreamVertex(0);
		EndStreamPrimitive(0);
		xfb_output_lod1 = pos;
		xfb_output_lod1.w = intensity;
		EmitStreamVertex(1);
		EndStreamPrimitive(1);
	}
	else if(distance > 0)
	{
		xfb_output_lod1 = pos;
		xfb_output_lod1.w = intensity;
		EmitStreamVertex(1);
	}
	
}