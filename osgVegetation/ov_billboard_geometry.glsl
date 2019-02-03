//#version 120
#version 330 compatibility
//#version 400
#extension GL_ARB_geometry_shader4 : enable
#pragma import_defines (BLT_ROTATED_QUAD, BLT_CROSS_QUADS, BLT_GRASS, SM_LISPSM, SM_VDSM1, SM_VDSM2, USE_LANDCOVER)

#if defined(SM_LISPSM) || defined(SM_VDSM1) || defined(SM_VDSM2)
 #define HAS_SHADOW
#endif

#ifdef BLT_GRASS
	#define MAX_NUM_VERTS 16
#else
	#define MAX_NUM_VERTS 8
#endif

layout(triangles) in;
layout(triangle_strip, max_vertices = MAX_NUM_VERTS) out;

in vec2 ov_te_texcoord[];
in vec3 ov_te_normal[];
out vec2 ov_geometry_texcoord;
out vec4 ov_geometry_color;
flat out int ov_geometry_tex_index;
flat out float ov_fade;
out vec3 ov_geometry_normal;
out float ov_depth;

uniform float osg_SimulationTime;
uniform sampler2D ov_color_texture;
#ifdef USE_LANDCOVER
uniform sampler2D ov_land_cover_texture;
uniform float ov_billboard_land_cover_id;
#endif
uniform int ov_num_billboards;
uniform vec4 ov_billboard_data[10];
uniform float ov_billboard_max_distance;
uniform float ov_billboard_color_threshold;
uniform float ov_billboard_color_impact;


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

void ov_shadow(vec4 ecPosition)
{
#ifdef HAS_SHADOW
#ifdef SM_LISPSM
	int shadowTextureUnit0 = shadowTextureUnit;
#endif
	//ecPosition = gl_ModelViewMatrix * ecPosition;
	// generate coords for shadow mapping                              
	gl_TexCoord[shadowTextureUnit0].s = dot(ecPosition, gl_EyePlaneS[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].t = dot(ecPosition, gl_EyePlaneT[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].p = dot(ecPosition, gl_EyePlaneR[shadowTextureUnit0]);
	gl_TexCoord[shadowTextureUnit0].q = dot(ecPosition, gl_EyePlaneQ[shadowTextureUnit0]);
#ifdef SM_VDSM2
	gl_TexCoord[shadowTextureUnit1].s = dot(ecPosition, gl_EyePlaneS[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].t = dot(ecPosition, gl_EyePlaneT[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].p = dot(ecPosition, gl_EyePlaneR[shadowTextureUnit1]);
	gl_TexCoord[shadowTextureUnit1].q = dot(ecPosition, gl_EyePlaneQ[shadowTextureUnit1]);
#endif
#endif	
}

float ov_range_rand(float minValue, float maxValue, vec2 co)
{
    float t = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
    return minValue + t*(maxValue-minValue);
}

float ov_rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Generate a pseudo-random barycentric point inside a triangle.
vec3 ov_get_random_barycentric_point(vec2 seed)
{
    vec3 b;
    b[0] = ov_rand(seed.xy);
    b[1] = ov_rand(seed.yx);
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
	vec4 random_pos = vec4(0,0,0,1);
    vec2 terrain_texcoord = vec2(0,0);
    vec3 terrain_normal = vec3(0,0,0);
	//gen a random point in triangle
    vec3 b = ov_get_random_barycentric_point(gl_PositionIn[0].xy);
    for(int i=0; i < 3; ++i)
    {
		random_pos.x += b[i] * gl_PositionIn[i].x;
		random_pos.y += b[i] * gl_PositionIn[i].y;
		random_pos.z += b[i] * gl_PositionIn[i].z;
        
		terrain_texcoord.x += b[i] * ov_te_texcoord[i].x;
		terrain_texcoord.y += b[i] * ov_te_texcoord[i].y;

		terrain_normal.x += b[i] * ov_te_normal[i].x;
		terrain_normal.y += b[i] * ov_te_normal[i].y;
		terrain_normal.z += b[i] * ov_te_normal[i].z;
    }
	
#ifdef USE_LANDCOVER
	vec4 lc_color = texture2D(ov_land_cover_texture, terrain_texcoord);
	//if (lc_color.y < ov_billboard_land_cover_id)
	//	return;

	if (lc_color.x > 0 || lc_color.z > 0)
		return;
	//if (ov_billboard_land_cover_id > 0.5 && lc_color.y < 0.5)
	//	return;
#endif

	ov_geometry_color = texture2D(ov_color_texture, terrain_texcoord);

	if(length(ov_geometry_color.xyz) > ov_billboard_color_threshold)
		return;

	//Scale color by some constant
	ov_geometry_color.xyz *= 1.9;

	ov_geometry_color.xyz = mix(vec3(1, 1, 1), ov_geometry_color.xyz, ov_billboard_color_impact);
	

	vec4 camera_pos = gl_ModelViewMatrixInverse[3];
	
	vec3 camera_to_bb_dir = camera_pos.xyz - random_pos.xyz;
	//we are only interested in xy-plane direction
	camera_to_bb_dir.z = 0;
	camera_to_bb_dir = normalize(camera_to_bb_dir);
	vec4 mv_pos = gl_ModelViewMatrix * random_pos;
	ov_depth = mv_pos.z;

	bool shadow_camera = true;
	if (gl_ProjectionMatrix[3][3] == 0)
		shadow_camera = false;

	float adjusted_max_dist = 10000;
	if (!shadow_camera)
		adjusted_max_dist = ov_billboard_max_distance*max(gl_ProjectionMatrix[0][0], 1);
	
	//just pick some fade distance
	float bb_fade_dist = adjusted_max_dist /3.0;
	float bb_fade_start_dist = adjusted_max_dist - bb_fade_dist;
	float bb_scale = 1 -clamp((-mv_pos.z - bb_fade_start_dist) / bb_fade_dist, 0.0, 1.0);
	
	ov_fade = bb_scale;
	//bb_scale = 1.0;

	//culling
    if (bb_scale == 0.0 )
        return;
	bb_scale = 1.0;

	
	float rand_scale = ov_range_rand(0.5, 1.5, random_pos.xy + vec2(10.5,111.3));
	bb_scale = bb_scale*bb_scale*bb_scale*rand_scale;
	
	//get random billboard
	float rand_val = ov_range_rand(0, 1.0, random_pos.xy + vec2(1.5,1.3));
	ov_geometry_tex_index = 0;
	float acc_probablity = 0;
	for (int i = 0; i < ov_num_billboards; i++)
	{
		float bb_probablity = ov_billboard_data[i].w;
		if (rand_val > acc_probablity && rand_val < acc_probablity + bb_probablity)
		{
			ov_geometry_tex_index = i;
			break;
		}
		acc_probablity += bb_probablity;
	}

	vec4 billboard_data = ov_billboard_data[ov_geometry_tex_index];
	float bb_half_width = bb_scale*billboard_data.x*0.5;
	float bb_height = bb_scale*billboard_data.y;
	float bb_intensity = billboard_data.z;
	ov_geometry_color.xyz = ov_geometry_color.xyz*bb_intensity;
	//ov_geometry_normal = gl_NormalMatrix * vec3(0, 0, 1);
	ov_geometry_normal = terrain_normal;

#ifdef BLT_ROTATED_QUAD

	if (!shadow_camera)
	{
		vec3 up_view = gl_NormalMatrix * vec3(0, 0, 1);
		vec3 tangent_vector = normalize(cross(vec3(0, 0, -1), up_view));
		vec3 bb_right = tangent_vector * bb_half_width;
		vec3 bb_up = up_view* bb_height;

		vec4 p0 = vec4(mv_pos.xyz - bb_right, 1.0);
		vec4 p1 = vec4(mv_pos.xyz + bb_right, 1.0);
		vec4 p2 = vec4(p0.xyz + bb_up, 1.0);
		vec4 p3 = vec4(p1.xyz + bb_up, 1.0);

		gl_Position = gl_ProjectionMatrix*p0;
		ov_shadow(p0);
		ov_geometry_texcoord = vec2(0,0);
		EmitVertex();
    
		gl_Position = gl_ProjectionMatrix*p1;
		ov_shadow(p1);
		ov_geometry_texcoord = vec2(1,0);
		EmitVertex();
    
		gl_Position = gl_ProjectionMatrix*p2;
		ov_shadow(p2);
		ov_geometry_texcoord = vec2(0,1);
		EmitVertex();

		gl_Position = gl_ProjectionMatrix*p3;
		ov_shadow(p3);
		ov_geometry_texcoord = vec2(1,1);
		EmitVertex();
		
		EndPrimitive();
	}
	else	
	{
		vec3 up_view = gl_NormalMatrix * vec3(0, 0, 1);
		vec3 tangent_vector = gl_NormalMatrix * vec3(1,0,0); // vector pointing east-ish.
		vec3 bb_right = cross(tangent_vector, up_view) * bb_half_width;
		vec3 bb_up = up_view * bb_height;
		
		for(int i=0; i<2; ++i)
		{
			vec4 p0 = vec4(mv_pos.xyz - bb_right, 1.0);
			vec4 p1 = vec4(mv_pos.xyz + bb_right, 1.0);
			vec4 p2 = vec4(p0.xyz + bb_up, 1.0);
			vec4 p3 = vec4(p1.xyz + bb_up, 1.0);

			gl_Position = gl_ProjectionMatrix*p0;
			ov_geometry_texcoord = vec2(0,0);
			EmitVertex();
		
			gl_Position = gl_ProjectionMatrix*p1;
			ov_geometry_texcoord = vec2(1,0);
			EmitVertex();
		
			gl_Position = gl_ProjectionMatrix*p2;
			ov_geometry_texcoord = vec2(0,1);
			EmitVertex();

			gl_Position = gl_ProjectionMatrix*p3;
			ov_geometry_texcoord = vec2(1,1);
			EmitVertex();
			
			EndPrimitive();

			tangent_vector = gl_NormalMatrix * vec3(0,1,0);
			bb_right = cross(tangent_vector, up_view) * bb_half_width;
		}

	
	}
#endif

#if defined(BLT_CROSS_QUADS) || defined(BLT_GRASS) 
	//Hack to support VPB terrain-tiles geometry that use scaling to get terrain vertices from normalized space. 
	//We extract the scale factor and use the inverse to scale the billboard vectors  
	vec3 inv_terrain_scale = vec3(1.0/length(gl_ModelViewMatrix[0].xyz), 1.0/length(gl_ModelViewMatrix[1].xyz), 1.0/length(gl_ModelViewMatrix[2].xyz));
#endif

#ifdef BLT_CROSS_QUADS
	//get fake random rotation in radians
	float rand_rad = mod(random_pos.x, 2 * 3.14);

	//calc sin and cos for random rotation
	float sin_rot = sin(rand_rad);
	float cos_rot = cos(rand_rad);

	//calc sin and cos part for billboard width
	float width_sin = bb_half_width * sin_rot;
	float width_cos = bb_half_width * cos_rot;
	float wind = (1 + sin(osg_SimulationTime * rand_rad))*0.1;
	float sin_wind = sin_rot*wind;
	float cos_wind = cos_rot*wind;

	//set billboard up vector
	vec3 bb_up = vec3(0, 0, bb_height) * inv_terrain_scale;

	//First quad, left vector used to offset in xy-space from anchor point 
	vec3 bb_left = vec3(-width_sin, -width_cos, 0.0) * inv_terrain_scale;

	//Calc a offset vector to add wind effect
	vec3 offset = vec3(sin_wind, cos_wind, 0) * inv_terrain_scale;

	vec4 bb_vertex;
	bb_vertex.w = random_pos.w;
	bb_vertex.xyz = random_pos.xyz + bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz + bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 1.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 1.0); EmitVertex();
	EndPrimitive();
	
	//Second quad, calc new left vector (perpendicular to prev left vec)
	bb_left = vec3(-width_cos, width_sin, 0.0) * inv_terrain_scale;
	//also recalc offset to reflect that we have a 90-deg rotatation
	offset = vec3(sin_wind, cos_wind, 0) * inv_terrain_scale;

	bb_vertex.xyz = random_pos.xyz + bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz + bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 1.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 1.0); EmitVertex();
	EndPrimitive();
#endif


#ifdef BLT_GRASS
	
	//get fake random rotation in radians
	float rand_rad = mod(random_pos.x, 2 * 3.14);
	
	//calc sin and cos for random rotation
	float sin_rot = sin(rand_rad);
	float cos_rot = cos(rand_rad);

	//calc sin and cos part for billboard width
	float width_sin = bb_half_width * sin_rot;
	float width_cos = bb_half_width * cos_rot;
	float wind = (1 + sin(osg_SimulationTime * rand_rad))*0.1;
	float sin_wind = sin_rot*wind;
	float cos_wind = cos_rot*wind;
	
	//set billboard up vector
	vec3 bb_up = vec3(0, 0, bb_height);

	//First quad, left vector used to offset in xy-space from anchor point 
	vec3 bb_left = vec3(-width_sin, -width_cos, 0.0) * inv_terrain_scale;

	//Calc a adhoc offset vector used to offset top verticies (perpendicular to left vec)
	//Here we just offset by the perpendicular left vector that will give us a 45-deg tilt
	//if bb height and width are the same. On top of that we add the wind effect.
	vec3 offset = (vec3(width_cos, -width_sin, 0) + vec3(sin_wind, cos_wind, 0)) * inv_terrain_scale;

	vec4 bb_vertex;
	bb_vertex.w = random_pos.w;
	bb_vertex.xyz = random_pos.xyz + bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz + bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 1.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 1.0); EmitVertex();
	EndPrimitive();

	//Second quad, just flip offset
	offset = -offset;

	bb_vertex.xyz = random_pos.xyz + bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz + bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 1.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 1.0); EmitVertex();
	EndPrimitive();

	//Third quad, calc new left vector (perpendicular to prev left vec)
	bb_left = vec3(-width_cos, width_sin, 0.0) * inv_terrain_scale;
	//also recalc offset to reflect that we have a 90-deg rotatation
	offset = (vec3(-width_sin, -width_cos, 0) + vec3(sin_wind, cos_wind, 0)) * inv_terrain_scale;

	bb_vertex.xyz = random_pos.xyz + bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz + bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 1.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 1.0); EmitVertex();
	EndPrimitive();

	//Last quad, just flip offset as before
	offset = -offset;
	bb_vertex.xyz = random_pos.xyz + bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left;                  gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 0.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz + bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(0.0, 1.0); EmitVertex();
	bb_vertex.xyz = random_pos.xyz - bb_left + bb_up + offset; gl_Position = gl_ModelViewProjectionMatrix * bb_vertex; ov_shadow(gl_ModelViewMatrix*bb_vertex); ov_geometry_texcoord = vec2(1.0, 1.0); EmitVertex();
	EndPrimitive();
#endif
}