#version 420 compatibility
#extension GL_ARB_geometry_shader4 : enable
#extension GL_ARB_enhanced_layouts : enable
layout(triangles) in;
layout(points, max_vertices = 1) out;

//#define OV_TERRAIN_COLOR_INT

in ov_VertexData
{
  vec4 Position;
  vec3 Normal;
  vec2 TexCoord0;
} ov_in[];

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform int ov_indirectCommandSize; // = sizeof(DrawArraysIndirectCommand) / sizeof(unsigned int) = 4
uniform int ov_numInstanceTypes;

layout(R32I) coherent uniform iimageBuffer ov_indirectCommand0;
layout(R32I) coherent uniform iimageBuffer ov_indirectCommand1;

layout(R32I) coherent uniform iimageBuffer ov_getIndirectCommand( int index )
{
	if(index==0) return ov_indirectCommand0;
	if(index==1) return ov_indirectCommand1;
	return ov_indirectCommand0;
}

layout(RGBA32F) coherent uniform imageBuffer ov_indirectTarget0;
layout(RGBA32F) coherent uniform imageBuffer ov_indirectTarget1;

layout(RGBA32F) coherent uniform imageBuffer ov_getIndirectTarget( int index )
{
	if(index==0) return ov_indirectTarget0;
	if(index==1) return ov_indirectTarget1;
	return ov_indirectTarget0;
}

struct InstanceLOD
{
	vec4 bbMin;
	vec4 bbMax;
	ivec4 indirectTargetParams;    // x=targetID, y=indexInTarget, z=offsetInTarget, w=lodMaxQuantity
	vec4 distances;               // x=minDistance, y=minFadeDistance, z=maxFadeDistance, w=maxDistance
};

const int OV_MAXIMUM_LOD_NUMBER = 8;

struct InstanceType
{
	vec4 bbMin;
	vec4 bbMax;
	vec4 floatParams;
	ivec4 params;
	InstanceLOD lods[OV_MAXIMUM_LOD_NUMBER];
};

layout(std140) uniform ov_instanceTypesData
{
	InstanceType instanceTypes[32];
};

bool ov_passFilter(vec3 terrain_pos, vec3 terrain_normal, vec2 terrain_texcoord);
vec4 ov_getTerrainColor(float depth, vec2 tex_coord0, vec2 terrain_pos);

bool ov_boundingBoxInViewFrustum( in mat4 matrix, in vec3 bb_min, in vec3 bb_max )
{
	vec4 bounding_box[8];
	bounding_box[0] = matrix * vec4( bb_max.x, bb_max.y, bb_max.z, 1.0);
	bounding_box[1] = matrix * vec4( bb_min.x, bb_max.y, bb_max.z, 1.0);
	bounding_box[2] = matrix * vec4( bb_max.x, bb_min.y, bb_max.z, 1.0);
	bounding_box[3] = matrix * vec4( bb_min.x, bb_min.y, bb_max.z, 1.0);
	bounding_box[4] = matrix * vec4( bb_max.x, bb_max.y, bb_min.z, 1.0);
	bounding_box[5] = matrix * vec4( bb_min.x, bb_max.y, bb_min.z, 1.0);
	bounding_box[6] = matrix * vec4( bb_max.x, bb_min.y, bb_min.z, 1.0);
	bounding_box[7] = matrix * vec4( bb_min.x, bb_min.y, bb_min.z, 1.0);

	int out_of_bound[6] = int[6]( 0, 0, 0, 0, 0, 0 );
	for (int i=0; i<8; i++)
	{
		out_of_bound[0] += int( bounding_box[i].x >  bounding_box[i].w );
		out_of_bound[1] += int( bounding_box[i].x < -bounding_box[i].w );
		out_of_bound[2] += int( bounding_box[i].y >  bounding_box[i].w );
		out_of_bound[3] += int( bounding_box[i].y < -bounding_box[i].w );
		out_of_bound[4] += int( bounding_box[i].z >  bounding_box[i].w );
		out_of_bound[5] += int( bounding_box[i].z < -bounding_box[i].w );
	}
	return (out_of_bound[0] < 8 ) && ( out_of_bound[1] < 8 ) && ( out_of_bound[2] < 8 ) && ( out_of_bound[3] < 8 ) && ( out_of_bound[4] < 8 ) && ( out_of_bound[5] < 8 );
}

float ov_rand(in vec2 seed)
{
	return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float ov_rangeRand(in float minValue, in float maxValue, in vec2 seed)
{
	float t = ov_rand(seed);
	return minValue + t*(maxValue-minValue);
}

int ov_getRandomMeshType(in vec2 seed)
{
	float rand_val = ov_rangeRand(0, 1.0, seed);
	int mesh_index = 0;
	float acc_probablity = 0;
	for (int i = 0; i < ov_numInstanceTypes; i++)
	{
		float mesh_probablity = instanceTypes[i].bbMin.w;
		if (rand_val > acc_probablity && rand_val < acc_probablity + mesh_probablity)
		{
			mesh_index = i;
			break;
		}
		acc_probablity += mesh_probablity;
	}
	return mesh_index;
}

// Generate a pseudo-random barycentric point inside a triangle.
vec3 ov_getRandomBarycentricPoint(vec2 seed)
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



void ov_getRandomPointInTriangle(out vec4 position, out vec3 normal, out vec2 tex_coords)
{
	position = vec4(0,0,0,1);
	normal = vec3(0,0,0);
	tex_coords = vec2(0,0);
	vec3 b = ov_getRandomBarycentricPoint(ov_in[0].Position.xy);
	for(int i=0; i < 3; ++i)
	{
		position.x += b[i] * ov_in[i].Position.x;
		position.y += b[i] * ov_in[i].Position.y;
		position.z += b[i] * ov_in[i].Position.z;

		normal.x += b[i] * ov_in[i].Normal.x;
		normal.y += b[i] * ov_in[i].Normal.y;
		normal.z += b[i] * ov_in[i].Normal.z;

		tex_coords.x += b[i] * ov_in[i].TexCoord0.x;
		tex_coords.y += b[i] * ov_in[i].TexCoord0.y;
	}
}

mat4 ov_getRotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
		oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
		oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
		0.0, 0.0, 0.0, 1.0);
}

mat4 ov_getTransformationMatrix(vec4 position, float scale, float scale_variation)
{
	float rand_val = ov_rand(position.xy + vec2(10.2,3.5));
	float rand_angle = rand_val*3.14*0.5;
	float rand_scale = scale*((1.0 - scale_variation * 0.5) + rand_val * scale_variation);
	mat4 rand_rot_mat = ov_getRotationMatrix(vec3(0, 0, 1), rand_angle);
	mat4 rand_scale_mat = mat4(rand_scale, 0, 0, 0,
						  0, rand_scale, 0, 0,
						  0, 0, rand_scale, 0,
						  0, 0, 0, 1);
	mat4 final_trans = rand_rot_mat*rand_scale_mat;
	final_trans[3] = position;
	return final_trans;
}

void main(void) 
{
	//get a random position inside current triangle
	vec4 instance_position;
	vec2 instance_tex_coords;
	vec3 instance_normal;
	ov_getRandomPointInTriangle(instance_position, instance_normal, instance_tex_coords);
	
	//check if this is valid location
	if(!ov_passFilter(instance_position.xyz, instance_normal, instance_tex_coords))
		return;
	 
	//get random mesh
	int instance_type_id = ov_getRandomMeshType(instance_position.xy);
	
	//get transformation with random rotation and scale
	float scale_variation = instanceTypes[instance_type_id].floatParams.y;
	float scale = instanceTypes[instance_type_id].floatParams.w;
	mat4 instance_matrix = ov_getTransformationMatrix(instance_position, scale, scale_variation);
	mat4 mvpo_matrix = osg_ModelViewProjectionMatrix * instance_matrix;

	// gl_Position is created only for debugging purposes
	//gl_Position = mvpo_matrix * vec4(0.0,0.0,10.0,1.0);
	//EmitVertex();
	//EndPrimitive();
   
	if( ov_boundingBoxInViewFrustum( mvpo_matrix, instanceTypes[instance_type_id].bbMin.xyz, instanceTypes[instance_type_id].bbMax.xyz ) )
	{
		//ov_color = vec4(1.0,0.0,0.0,1.0);
		vec4 mv_pos = osg_ModelViewMatrix * instance_position;
		float distance_to_object = length(mv_pos.xyz);
		int max_lods = instanceTypes[instance_type_id].params.x;

		bool shadow_camera = osg_ProjectionMatrix[3][3] != 0;
		
		int start_lod = 0;
		float lod_scale = 1.0;
		if (shadow_camera)
		{
			//select last lod
			start_lod = max(0, max_lods - 1);
			//and override object distance to be inside lod-range
			distance_to_object = instanceTypes[instance_type_id].lods[start_lod].distances.y + 0.1; //note that we add 0.1m to avoid problems with zero values
		}
		else
		{
			//scale dist by fov
			lod_scale = max(osg_ProjectionMatrix[0][0], 1);
		}
#ifdef OV_TERRAIN_COLOR_INT
		vec3 terrain_color =  ov_getTerrainColor(-mv_pos.z, instance_tex_coords.xy, instance_position.xy).xyz;
#endif
		vec4 max_lod_dist = instanceTypes[instance_type_id].lods[max_lods-1].distances;

		for(int i = start_lod ; i < max_lods; ++i)
		{
			vec4 lod_dist = instanceTypes[instance_type_id].lods[i].distances;
			lod_dist = lod_dist * lod_scale;

			if(ov_boundingBoxInViewFrustum( mvpo_matrix, instanceTypes[instance_type_id].lods[i].bbMin.xyz, instanceTypes[instance_type_id].lods[i].bbMax.xyz ) &&
				(distance_to_object >= lod_dist.x ) && ( distance_to_object < lod_dist.w ))
			{
#ifdef OV_TERRAIN_COLOR_INT				
				const vec3 all_ones = vec3(1.0);
				float intensity = 0.2 + 4 * dot(terrain_color, all_ones)/3.0;
#else
				//get intensity
				float intensity = instanceTypes[instance_type_id].lods[i].bbMin.w;
				float max_intensity_variation = instanceTypes[instance_type_id].floatParams.x;
				intensity = (intensity - max_intensity_variation*0.5) + max_intensity_variation*ov_rand(instance_position.xy + vec2(20.1,6.5));
#endif
				float fade_alpha  = ( clamp( (distance_to_object - lod_dist.x)/(lod_dist.y - lod_dist.x), 0.0, 1.0 ) 
					-  clamp( (distance_to_object- lod_dist.z)/( lod_dist.w - lod_dist.z), 0.0, 1.0 ) );
				
				int indirect_command_index   = instanceTypes[instance_type_id].lods[i].indirectTargetParams.x;
				int indirect_command_address = instanceTypes[instance_type_id].lods[i].indirectTargetParams.y;
				int object_index             = imageAtomicAdd( ov_getIndirectCommand( indirect_command_index ), indirect_command_address * ov_indirectCommandSize + 1, 1 );
				int indirect_target_address  = 6*(instanceTypes[instance_type_id].lods[i].indirectTargetParams.z + object_index);
				imageStore( ov_getIndirectTarget(indirect_command_index), indirect_target_address + 0, instance_matrix[0] );
				imageStore( ov_getIndirectTarget(indirect_command_index), indirect_target_address + 1, instance_matrix[1] );
				imageStore( ov_getIndirectTarget(indirect_command_index), indirect_target_address + 2, instance_matrix[2] );
				imageStore( ov_getIndirectTarget(indirect_command_index), indirect_target_address + 3, instance_matrix[3] );
				vec4 extra_params = vec4(intensity,i,0,0);
				imageStore( ov_getIndirectTarget(indirect_command_index), indirect_target_address + 4, extra_params);
				 vec4 _IdParams = vec4(instance_type_id,0,0,0);
				imageStore( ov_getIndirectTarget(indirect_command_index), indirect_target_address + 5, vec4(_IdParams.x,_IdParams.y,float(fade_alpha),0.0) );
				//EmitVertex();
				//EndPrimitive();
			}
		}
	}
}