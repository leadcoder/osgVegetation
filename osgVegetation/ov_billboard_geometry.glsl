//#version 120
#version 330 compatibility
//compatibility
//#version 400
#extension GL_ARB_geometry_shader4 : enable
#pragma import_defines (BLT_ROTATED_QUAD, BLT_CROSS_QUADS, BLT_GRASS)

#ifdef BLT_GRASS
	#define MAX_NUM_VERTS 16
#else
	#define MAX_NUM_VERTS 8
#endif

layout(triangles) in;
layout(triangle_strip, max_vertices = MAX_NUM_VERTS) out;

in ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_in[];

out ov_BillboardVertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
  vec4 Color;
  float Depth;
  flat int TextureArrayIndex;
  flat float Fade;
} ov_bb_out;

uniform mat4 osg_ProjectionMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;

uniform float osg_SimulationTime;

#ifdef OV_NOISE_TEXTURE
uniform sampler2D ov_noise_texture;
#endif
uniform int ov_num_billboards;
uniform vec4 ov_billboard_data[10];
uniform float ov_billboard_max_distance;

//forward declaration
void ov_setShadowTexCoords(vec4 mv_pos);
vec4 ov_applyTerrainElevation(vec4 pos, vec2 tex_coords);
vec3 ov_applyTerrainNormal(vec3 normal, vec2 tex_coords);
vec4 ov_getTerrainColor(float depth, vec2 tex_coord0, vec2 terrain_pos);
bool ov_passFilter(vec3 terrain_pos, vec3 terrain_normal, vec2 terrain_texcoord);

float ov_random(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float ov_randomRange(float minValue, float maxValue, vec2 co)
{
    float t = ov_random(co);//fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
    return minValue + t * (maxValue - minValue);
}

// Generate a pseudo-random barycentric point inside a triangle.
vec3 ov_getRandomBarycentricCoords(vec2 seed)
{
    vec3 b;
    b[0] = ov_random(seed.xy);
    b[1] = ov_random(seed.yx);
    if (b[0]+b[1] >= 1.0)
    {
        b[0] = 1.0 - b[0];
        b[1] = 1.0 - b[1];
    }
    b[2] = 1.0 - b[0] - b[1];
    return b;
}
/*
void ov_getRandomPointInTriangle(in vec4 tri_vertices[3],in vec2 tri_tex_coords[3],in vec3 tri_normals[3],  out vec4 point, out vec2 point_tex_coords, out vec3 point_normal)
{
	point = vec4(0,0,0,1);
	point_tex_coords = vec2(0,0);
	vec3 b = ov_get_random_barycentric_point(tri_vertices[0].xy);
    for(int i=0; i < 3; ++i)
    {
		point.x += b[i] * tri_vertices[i].x;
		point.y += b[i] * tri_vertices[i].y;
		point.z += b[i] * tri_vertices[i].z;

		point_tex_coords.x += b[i] * tri_tex_coords[i].x;
        point_tex_coords.y += b[i] * tri_tex_coords[i].y;

		point_normal.x += b[i] * tri_normals[i].x;
		point_normal.y += b[i] * tri_normals[i].y;
		point_normal.z += b[i] * tri_normals[i].z;
    }
}
*/

float ov_calculateFade(float depth, float max_dist)
{
	//just pick some fade distance for now
	float fade_dist = max_dist / 3.0;
	float fade_start_dist = max_dist - fade_dist;
	return 1.0 - clamp((depth - fade_start_dist) / fade_dist, 0.0, 1.0);
}

int ov_getRandomBillboardIndex(vec2 seed)
{
    float rand_val = ov_random(seed);
	int index = 0;
	float acc_probablity = 0;
	for (int i = 0; i < ov_num_billboards; i++)
	{
		float probablity = ov_billboard_data[i].w;
		if (rand_val > acc_probablity && rand_val < acc_probablity + probablity)
		{
			index = i;
			break;
		}
		acc_probablity += probablity;
	}
	return index;
}

mat3 ov_getRotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return mat3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
		oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
		oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c);
}

#ifdef BLT_ROTATED_QUAD

void ov_emitRotatedBillboard(vec3 mv_pos, float bb_half_width, float bb_height)
{
	vec3 up_view = osg_NormalMatrix * vec3(0, 0, 1);
	vec3 tangent_vector = normalize(cross(vec3(0, 0, -1), up_view));
	vec3 bb_right = tangent_vector * bb_half_width;
	vec3 bb_up = up_view* bb_height;

	vec4 p0 = vec4(mv_pos.xyz - bb_right, 1.0);
	vec4 p1 = vec4(mv_pos.xyz + bb_right, 1.0);
	vec4 p2 = vec4(p0.xyz + bb_up, 1.0);
	vec4 p3 = vec4(p1.xyz + bb_up, 1.0);

	gl_Position = osg_ProjectionMatrix*p0;
	ov_setShadowTexCoords(p0);
	ov_bb_out.TexCoord0 = vec2(0,0);
	EmitVertex();
    
	gl_Position = osg_ProjectionMatrix*p1;
	ov_setShadowTexCoords(p1);
	ov_bb_out.TexCoord0 = vec2(1,0);
	EmitVertex();
    
	gl_Position = osg_ProjectionMatrix*p2;
	ov_setShadowTexCoords(p2);
	ov_bb_out.TexCoord0 = vec2(0,1);
	EmitVertex();

	gl_Position = osg_ProjectionMatrix*p3;
	ov_setShadowTexCoords(p3);
	ov_bb_out.TexCoord0 = vec2(1,1);
	EmitVertex();

	EndPrimitive();
}

#endif


void ov_emitCrossQuadsMV(vec3 mv_pos, mat3 model_rot_mat, float wind, float bb_half_width, float bb_height)
{
	mat3 rot_mat = osg_NormalMatrix * model_rot_mat;
	vec3 up_view = rot_mat * vec3(0, 0, 1);
	vec3 tangent_vector = rot_mat * vec3(1,0,0);
	vec3 bb_right = cross(tangent_vector, up_view) * bb_half_width;
	vec3 bb_up = up_view * bb_height;
	vec3 wind_vec = normalize(bb_right) * wind;
	
	for(int i=0; i<2; ++i)
	{
		vec4 p0 = vec4(mv_pos.xyz - bb_right, 1.0);
		vec4 p1 = vec4(mv_pos.xyz + bb_right, 1.0);
		vec4 p2 = vec4(p0.xyz + bb_up, 1.0);
		vec4 p3 = vec4(p1.xyz + bb_up, 1.0);
		p2.xyz += wind_vec;
		p3.xyz += wind_vec;

		gl_Position = osg_ProjectionMatrix * p0;
		ov_bb_out.TexCoord0 = vec2(0,0);
		EmitVertex();
		
		gl_Position = osg_ProjectionMatrix*p1;
		ov_bb_out.TexCoord0 = vec2(1,0);
		EmitVertex();
		
		gl_Position = osg_ProjectionMatrix*p2;
		ov_bb_out.TexCoord0 = vec2(0,1);
		EmitVertex();

		gl_Position = osg_ProjectionMatrix*p3;
		ov_bb_out.TexCoord0 = vec2(1,1);
		EmitVertex();
			
		EndPrimitive();

		tangent_vector = rot_mat * vec3(0,1,0);
		bb_right = cross(tangent_vector, up_view) * bb_half_width;
	}
}

void ov_emitCrossQuads(vec4 pos, mat3 rot, vec3 wind_vec, float bb_half_width, float bb_height)
{
	//set billboard up vector
	vec3 bb_up = vec3(0, 0, bb_height);

	//First quad, left vector used to offset in xy-space from anchor point 
	vec3 bb_left = rot * vec3(bb_half_width, 0.0, 0.0);

	for(int i=0; i<2; ++i)
	{
		vec4 p0 = vec4(pos.xyz + bb_left, pos.w);
		vec4 p1 = vec4(pos.xyz - bb_left, pos.w);
		vec4 p2 = vec4(p0.xyz + bb_up, pos.w);
		vec4 p3 = vec4(p1.xyz + bb_up, pos.w);

		p2.xyz += wind_vec;
		p3.xyz += wind_vec;

		gl_Position = osg_ModelViewProjectionMatrix * p0; 
		ov_setShadowTexCoords(osg_ModelViewMatrix * p0); 
		ov_bb_out.TexCoord0 = vec2(0.0, 0.0); 
		EmitVertex();

		gl_Position = osg_ModelViewProjectionMatrix * p1; 
		ov_setShadowTexCoords(osg_ModelViewMatrix * p1); 
		ov_bb_out.TexCoord0 = vec2(1.0, 0.0); 
		EmitVertex();

		gl_Position = osg_ModelViewProjectionMatrix * p2; 
		ov_setShadowTexCoords(osg_ModelViewMatrix * p2); 
		ov_bb_out.TexCoord0 = vec2(0.0, 1.0); 
		EmitVertex();

		gl_Position = osg_ModelViewProjectionMatrix * p3; 
		ov_setShadowTexCoords(osg_ModelViewMatrix * p3); 
		ov_bb_out.TexCoord0 = vec2(1.0, 1.0); 
		EmitVertex();

		EndPrimitive();
	
		//Second quad, calc new left vector (perpendicular to prev left vec)
		bb_left = rot * vec3(0, bb_half_width, 0.0);
	}
}

void ov_emitGrass(vec4 pos, mat3 rot, vec3 wind_vec, float bb_half_width, float bb_height)
{
	//set billboard up vector
	vec3 bb_up = vec3(0, 0, bb_height);

	//First quad, left vector used to offset in xy-space from anchor point 
	vec3 bb_left = rot * vec3(bb_half_width, 0.0, 0.0);
	vec3 bb_left_second = rot * vec3(0, bb_half_width, 0.0);
	//Calc a offset vector to add wind effect
	vec3 offset_vec = bb_left_second;
	vec3 offset_vec_second = bb_left;

	for(int i=0; i < 2; ++i)
	{
		for(int j=0; j < 2; ++j)
		{
			vec4 p0 = vec4(pos.xyz + bb_left, pos.w);
			vec4 p1 = vec4(pos.xyz - bb_left, pos.w);
			vec4 p2 = vec4(p0.xyz + bb_up, pos.w);
			vec4 p3 = vec4(p1.xyz + bb_up, pos.w);

			p2.xyz += (wind_vec + offset_vec);
			p3.xyz += (wind_vec + offset_vec);

			gl_Position = osg_ModelViewProjectionMatrix * p0; 
			ov_setShadowTexCoords(osg_ModelViewMatrix*p0); 
			ov_bb_out.TexCoord0 = vec2(0.0, 0.0); 
			EmitVertex();

			gl_Position = osg_ModelViewProjectionMatrix * p1; 
			ov_setShadowTexCoords(osg_ModelViewMatrix*p1); 
			ov_bb_out.TexCoord0 = vec2(1.0, 0.0); 
			EmitVertex();

			gl_Position = osg_ModelViewProjectionMatrix * p2; 
			ov_setShadowTexCoords(osg_ModelViewMatrix*p2); 
			ov_bb_out.TexCoord0 = vec2(0.0, 1.0); 
			EmitVertex();

			gl_Position = osg_ModelViewProjectionMatrix * p3; 
			ov_setShadowTexCoords(osg_ModelViewMatrix*p3); 
			ov_bb_out.TexCoord0 = vec2(1.0, 1.0); 
			EmitVertex();

			EndPrimitive();

			offset_vec = -offset_vec;
		}
		//Second quad, calc new left vector (perpendicular to prev left vec)
		bb_left = bb_left_second;
		offset_vec = offset_vec_second;
	}
}

void main(void)
{
	vec4 terrain_pos = vec4(0,0,0,1);
    vec2 terrain_texcoord = vec2(0,0);
    vec3 terrain_normal = vec3(0,0,0);
	//vec4 terrain_normal2[3];
	//ov_getRandomPointInTriangle(gl_PositionIn, ov_te_texcoord, ov_te_normal, terrain_pos, terrain_texcoord, terrain_normal);
	//gen a random point in triangle
    vec3 barycentric_coords = ov_getRandomBarycentricCoords(ov_in[0].Position.xy);
    for(int i=0; i < 3; ++i)
    {
		terrain_pos.x += barycentric_coords[i] * ov_in[i].Position.x;
		terrain_pos.y += barycentric_coords[i] * ov_in[i].Position.y;
		terrain_pos.z += barycentric_coords[i] * ov_in[i].Position.z;
        
		terrain_texcoord.x += barycentric_coords[i] * ov_in[i].TexCoord0.x;
		terrain_texcoord.y += barycentric_coords[i] * ov_in[i].TexCoord0.y;

		terrain_normal.x += barycentric_coords[i] * ov_in[i].Normal.x;
		terrain_normal.y += barycentric_coords[i] * ov_in[i].Normal.y;
		terrain_normal.z += barycentric_coords[i] * ov_in[i].Normal.z;
    }
	terrain_pos = ov_applyTerrainElevation(terrain_pos, terrain_texcoord.xy);
	terrain_normal = ov_applyTerrainNormal(terrain_normal, terrain_texcoord.xy);

	//check if this is valid location
	if(!ov_passFilter(terrain_pos.xyz, terrain_normal, terrain_texcoord))
		return;
	
	vec4 mv_pos = osg_ModelViewMatrix * terrain_pos;
	ov_bb_out.Depth = -mv_pos.z;

	vec3 terrain_color =  ov_getTerrainColor(ov_bb_out.Depth, terrain_texcoord.xy, terrain_pos.xy).xyz;
	terrain_color += ov_getTerrainColor(ov_bb_out.Depth, terrain_texcoord.xy, terrain_pos.xy + vec2(0.4,0.4)).xyz;
	terrain_color += ov_getTerrainColor(ov_bb_out.Depth, terrain_texcoord.xy, terrain_pos.xy + vec2(-0.4,-0.4)).xyz;
	terrain_color = terrain_color*0.33;
	bool ortho_camera = true;
	if (osg_ProjectionMatrix[3][3] == 0)
		ortho_camera = false;

	float adjusted_max_dist = 10000;
	if (!ortho_camera)
		adjusted_max_dist = ov_billboard_max_distance * max(osg_ProjectionMatrix[0][0], 1);
	
	ov_bb_out.Fade = ov_calculateFade(ov_bb_out.Depth, adjusted_max_dist);

	//culling
    if (ov_bb_out.Fade == 0.0 )
        return;
	
	vec2 scale_seed = terrain_pos.xy + vec2(10.5,111.3);
	float base_scale = ov_randomRange(0.5, 1.5, scale_seed);
	
	//if we dont have multisample support, we fade by scale
	//base_scale = ov_fade * ov_fade * ov_fade * base_scale;
	
	//get random billboard
	vec2 index_seed = terrain_pos.xy + vec2(1.5, 1.3);
	ov_bb_out.TextureArrayIndex = ov_getRandomBillboardIndex(index_seed);
	
	vec4 billboard_data = ov_billboard_data[ov_bb_out.TextureArrayIndex];
	float bb_half_width = base_scale * billboard_data.x*0.5;
	float bb_height = base_scale * billboard_data.y;
	float bb_intensity = billboard_data.z;

	//bb_intensity += 0.3 * texture2D(ov_noise_texture, terrain_pos.xy * 0.004).x;
	
	ov_bb_out.Color.xyz  = terrain_color;
	ov_bb_out.Color.w = bb_intensity; 
	ov_bb_out.Normal = osg_NormalMatrix * terrain_normal;

#ifdef BLT_ROTATED_QUAD
	ov_emitRotatedBillboard(mv_pos.xyz, bb_half_width, bb_height);
#else //BLT_CROSS_QUADS || BLT_GRASS
    float random_rot = mod(terrain_pos.x*100, 2 * 3.14);
	mat3 rot_mat = ov_getRotationMatrix(vec3(0,0,1), random_rot);

#if 0
	//Hack to support VPB terrain-tiles geometry that use scaling to get terrain vertices from normalized space. 
	//We extract the scale factor and use the inverse to scale the billboard vectors  
	vec3 inv_terrain_scale = vec3(1.0 / length(osg_ModelViewMatrix[0].xyz), 1.0 / length(osg_ModelViewMatrix[1].xyz), 1.0 / length(osg_ModelViewMatrix[2].xyz));
	mat3 scale_mat = mat3(inv_terrain_scale.x, 0,                   0,   // first column
	                      0,                   inv_terrain_scale.y, 0,
	                      0,                   0,                   inv_terrain_scale.z);
	rot_mat = rot_mat*scale_mat;
#endif

	float wind_effect = sin(osg_SimulationTime * random_rot) * bb_height * 0.01;
    vec3 wind_vec = rot_mat * vec3(wind_effect,0,0);

#if defined(BLT_CROSS_QUADS)
	ov_emitCrossQuads(terrain_pos, rot_mat, wind_vec, bb_half_width, bb_height);
#elif defined(BLT_GRASS)
	ov_emitGrass(terrain_pos, rot_mat, wind_vec, bb_half_width, bb_height);
#endif

#endif

}