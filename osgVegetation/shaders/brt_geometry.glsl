#version 120
#pragma import_defines ( SM_LISPSM,SM_VDSM1,SM_VDSM2,CAST_SHADOW,BT_ROTATED_QUAD,BT_GRASS,TERRAIN_NORMAL)
#extension GL_EXT_geometry_shader4 : enable
#pragma osgveg
uniform float TileRadius; 
uniform float osg_SimulationTime;
#if defined(SM_LISPSM) || defined(SM_VDSM1) || defined(SM_VDSM1)
	#define HAS_SHADOW
#endif

#ifdef SM_LISPSM
	uniform int shadowTextureUnit;
#endif

#ifdef SM_VDSM1
	uniform int shadowTextureUnit0;
#endif

#ifdef SM_VDSM2
	uniform int shadowTextureUnit0;
	uniform int shadowTextureUnit1;
#endif	
 
varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 Color; 
varying float TextureIndex; 


void DynamicShadow(vec4 ecPosition )                               
{
#ifdef HAS_SHADOW
	#ifdef SM_LISPSM
		int shadowTextureUnit0 = shadowTextureUnit;
	#endif
		ecPosition = gl_ModelViewMatrix * ecPosition;						
		// generate coords for shadow mapping                              
		gl_TexCoord[shadowTextureUnit0].s = dot( ecPosition, gl_EyePlaneS[shadowTextureUnit0] );             
		gl_TexCoord[shadowTextureUnit0].t = dot( ecPosition, gl_EyePlaneT[shadowTextureUnit0] );             
		gl_TexCoord[shadowTextureUnit0].p = dot( ecPosition, gl_EyePlaneR[shadowTextureUnit0] );             
		gl_TexCoord[shadowTextureUnit0].q = dot( ecPosition, gl_EyePlaneQ[shadowTextureUnit0] );             
	#ifdef SM_VDSM2
		gl_TexCoord[shadowTextureUnit1].s = dot( ecPosition, gl_EyePlaneS[shadowTextureUnit1] );
		gl_TexCoord[shadowTextureUnit1].t = dot( ecPosition, gl_EyePlaneT[shadowTextureUnit1] );
		gl_TexCoord[shadowTextureUnit1].p = dot( ecPosition, gl_EyePlaneR[shadowTextureUnit1] );
		gl_TexCoord[shadowTextureUnit1].q = dot( ecPosition, gl_EyePlaneQ[shadowTextureUnit1] );
	#endif
#endif	
} 

void main(void)
{
    vec4 pos = gl_PositionIn[0];
    vec4 info = gl_PositionIn[1];
    vec4 info2 = gl_PositionIn[2];
    TextureIndex = info.z;
    Color = info2.xyz;
    vec2 scale = info.xy;
    scale.x *= 0.5;
    vec4 camera_pos = gl_ModelViewMatrixInverse[3];
#ifndef CAST_SHADOW
    float distance = length(camera_pos.xyz - pos.xyz);
    scale = scale*clamp((1.0 - (distance-TileRadius))/(TileRadius*0.2),0.0,1.0);
#endif
    vec4 e;
    e.w = pos.w;

#ifdef BT_ROTATED_QUAD
	vec3 dir = camera_pos.xyz - pos.xyz;
	//we are only interested in xy-plane direction
	dir.z = 0;
	dir = normalize(dir);
	vec3 up   = vec3(0.0, 0.0, 1.0*scale.y);//Up direction in OSG
	//vec3 left = cross(dir,up); //Generate billboard base vector
	
	vec3 left = vec3(-dir.y,dir.x, 0);
	left = normalize(left);
	left.xy *= scale.xx;
	#ifdef TERRAIN_NORMAL	
		vec3 n1 = vec3(0.0, 1.0, 0.0);
		vec3 n2=n1;
		vec3 n3=n1;
		vec3 n4=n1;
	#else
		float n_offset = 1.0;
		vec3 n1 = vec3( n_offset,0.0,1.0);
		vec3 n2 = vec3(-n_offset,0.0,1.0);
		vec3 n3 = n1;
		vec3 n4 = n2;
	#endif
		e.xyz =  pos.xyz + left;       gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,0.0); DynamicShadow(e); Normal = n1; EmitVertex();
		e.xyz =  pos.xyz - left;       gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,0.0); DynamicShadow(e); Normal = n2; EmitVertex();
		e.xyz =  pos.xyz + left + up;  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,1.0); DynamicShadow(e); Normal = n3; EmitVertex();
		e.xyz =  pos.xyz - left + up;  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,1.0); DynamicShadow(e); Normal = n4; EmitVertex();
		EndPrimitive();
#elif defined (BT_GRASS)
    float rand_rad = mod(pos.x, 2*3.14);
    float sw = scale.x*sin(rand_rad);
    float cw = scale.x*cos(rand_rad);
	float wind = (1 + sin(osg_SimulationTime*2))*0.1;
	float wx = sin(rand_rad)*wind;
	float wy = cos(rand_rad)*wind;
	vec4 offset1 = vec4(cw,-sw,0,0.0) + vec4(wx,wy,0,0.0);
	vec4 offset2 = vec4(-sw,-cw,0,0.0) + vec4(wx,wy,0,0.0);
    float h = scale.y;
    #ifdef TERRAIN_NORMAL
		vec3 n = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0));
	#else
		vec3 n = vec3(0.0,0.0,1.0);
	#endif
		e = pos + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();
		e = pos + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();
		e = pos + offset1 + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();
		e = pos + offset1 + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();
		EndPrimitive();
		
		e = pos + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();
		e = pos + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();
		e = pos - offset1 + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();
		e = pos - offset1 + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();
		EndPrimitive();
		
		e = pos + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();
		e = pos + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();
		e = pos + offset2 + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();
		e = pos + offset2 + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();
		EndPrimitive();
		
		e = pos + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();
		e = pos + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();
		e = pos - offset2 + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();
		e = pos - offset2 + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();
		EndPrimitive();
	
#else
    float rand_rad = mod(pos.x, 2*3.14);
    float sw = scale.x*sin(rand_rad);
    float cw = scale.x*cos(rand_rad);
	float wind = (1 + sin(osg_SimulationTime*2))*0.1;
	float wx = sin(rand_rad)*wind;
	float wy = cos(rand_rad)*wind;
	vec4 offset1 = vec4(wx,wy,0,0.0);
	vec4 offset2 = vec4(wx,wy,0,0.0);
    float h = scale.y;
    #ifdef TERRAIN_NORMAL
		vec3 n = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0));
	#else
		vec3 n = vec3(0.0,0.0,1.0);
	#endif
		e = pos + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();
		e = pos + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();
		e = pos + offset1 + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();
		e = pos + offset1 + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();
		EndPrimitive();
		
		e = pos + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();
		e = pos + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();
		e = pos + offset2 + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();
		e = pos + offset2 + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();
		EndPrimitive();
	
#endif
}