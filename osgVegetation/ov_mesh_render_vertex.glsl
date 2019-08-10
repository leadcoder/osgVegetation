 #version 420 compatibility
    
layout(location = 0) in vec4 ov_vertexPosition;
layout(location = 2) in vec3 ov_vertexNormal;
layout(location = 3) in vec4 ov_vertexColor;
layout(location = 8) in vec2 ov_vertexTexCoord0;
layout(location = 9) in vec3 ov_vertexTexCoord1;
    
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
    ivec4 params;
    InstanceLOD lods[OV_MAXIMUM_LOD_NUMBER];
};
    
layout(std140) uniform ov_instanceTypesData
{
    InstanceType instanceTypes[32];
};
    
layout(RGBA32F) coherent uniform imageBuffer ov_indirectTarget;

void ov_setShadowTexCoords(vec4 mv_pos);
    
out vec3 ov_position;
out vec3 ov_vPosition;
out vec3 ov_normal;
out vec2 ov_texCoord;
out vec4 ov_color;
flat out mat3 ov_texMat;
flat out float ov_fade;
flat out float ov_textureIndex;
flat out int ov_type;
      
void main()
{
    // every vertex has its type coded on VertexTexCoord1.x,
    // and its lodNumber coded in VertexTexCoord1.y
    int instance_type_index = int(ov_vertexTexCoord1.x);
    int instance_lod_number = int(ov_vertexTexCoord1.y);
    int indirect_target_address  = 6*(instanceTypes[instance_type_index].lods[instance_lod_number].indirectTargetParams.z + gl_InstanceID);
    mat4 instance_matrix = mat4(
        imageLoad(ov_indirectTarget, indirect_target_address + 0),
        imageLoad(ov_indirectTarget, indirect_target_address + 1),
        imageLoad(ov_indirectTarget, indirect_target_address + 2),
        imageLoad(ov_indirectTarget, indirect_target_address + 3) );
    vec4 extra_params = imageLoad(ov_indirectTarget, indirect_target_address + 4);
    vec4 id_params = imageLoad(ov_indirectTarget, indirect_target_address + 5);
    
	ov_fade = id_params.z;
	ov_type = instanceTypes[instance_type_index].lods[instance_lod_number].indirectTargetParams.w;
    
    mat4 mv_matrix = gl_ModelViewMatrix * instance_matrix;
       
    // Do fixed functionality vertex transform
    vec4  mv_pos  = mv_matrix * vec4(ov_vertexPosition.xyz,1.0);
    gl_Position = gl_ProjectionMatrix * mv_pos;
	ov_setShadowTexCoords(mv_pos);
    
	ov_vPosition = ov_vertexPosition.xyz;
    ov_position = mv_pos.xyz / mv_pos.w;
    ov_texMat = mat3(mv_matrix);
	//ov_normal = normalize( mat3(mv_matrix) * ov_vertexNormal.xyz );
	ov_normal = ov_vertexNormal.xyz;
	
    ov_texCoord = ov_vertexTexCoord0.xy;
	ov_textureIndex = ov_vertexTexCoord1.z;
	
	ov_color.xyz = ov_vertexColor.xyz * extra_params.x;
    ov_color.a = ov_vertexColor.a;
}