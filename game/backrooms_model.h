#pragma once

#include "backrooms_common.h"
#include "backrooms_rhi.h"

#include <string>
#include <vector>

struct mesh_vertex
{
    hmm_vec3 Position;
    hmm_vec2 UV;
    hmm_vec3 Normals;
    hmm_vec3 Tangent;
    hmm_vec3 Bitangent;
};

struct material_data
{
    hmm_vec3 AlbedoFactor;
    float MetallicFactor;
    float RoughnessFactor;
    hmm_vec3 Pad;
};

struct gltf_material
{
    std::string AlbedoPath;
    std::string NormalPath;
    std::string PBRPath;

    bool HasNormalMap;
    bool HasPBRMap;

    material_data MaterialData;

    rhi_image AlbedoImage;
    rhi_image NormalImage;
    rhi_image PBRImage;

    rhi_texture Albedo;
    rhi_texture Normal;
    rhi_texture PBR;
    rhi_buffer MaterialBuffer;
};

struct instance_data
{
    i32 PrimitiveIndex;
    hmm_vec3 Pad;
    hmm_vec4 BoundingSphere;
    hmm_mat4 Transform;
};

struct gltf_primitive
{
    rhi_buffer VertexBuffer;
    rhi_buffer IndexBuffer;
    rhi_buffer InstanceBuffer;

    u32 VertexBufferSize;
    u32 IndexBufferSize;
    
    u32 VertexCount;
    u32 IndexCount;
    u32 TriangleCount;
    u32 MaterialIndex;

    instance_data InstanceData;
};

struct gpu_mesh
{
    std::vector<gltf_primitive> Primitives;
    std::vector<gltf_material> Materials;

    u32 TotalVertexCount;
    u32 TotalIndexCount;
    u32 TotalTriangleCount;
    std::string Directory;
};

void GpuMeshLoad(gpu_mesh* Mesh, const std::string& Path);
void GpuMeshFree(gpu_mesh* Mesh);
