#include "backrooms_model.h"

#include <cgltf/cgltf.h>
#include <assert.h>
#include <stdio.h>

#define CGLTFCall(Call) do { cgltf_result Result = (Call); assert(Result == cgltf_result_success); } while(0)

struct aabb
{
    hmm_vec3 Min;
    hmm_vec3 Max;
};

u32 CGLTFComponentSize(cgltf_component_type Type)
{
    switch (Type)
    {
        case cgltf_component_type_r_8:
        case cgltf_component_type_r_8u:
            return 1;
        case cgltf_component_type_r_16:
        case cgltf_component_type_r_16u:
            return 2;
        case cgltf_component_type_r_32u:
        case cgltf_component_type_r_32f:
            return 4;
    }

    assert(0);
    return 0;
}

u32 CGLTFComponentCount(cgltf_type Type)
{
    switch (Type)
    {
        case cgltf_type_scalar:
            return 1;
        case cgltf_type_vec2:
            return 2;
        case cgltf_type_vec3:
            return 3;
        case cgltf_type_vec4:
        case cgltf_type_mat2:
            return 4;
        case cgltf_type_mat3:
            return 9;
        case cgltf_type_mat4:
            return 16;
    }

    assert(0);
    return 0;
}

void* GetAccessorData(cgltf_accessor* Accessor, u32* ComponentSize, u32* ComponentCount)
{
    *ComponentSize = CGLTFComponentSize(Accessor->component_type);
    *ComponentCount = CGLTFComponentCount(Accessor->type);

    cgltf_buffer_view* View = Accessor->buffer_view;
    return OFFSET_PTR_BYTES(void, View->buffer->data, View->offset);
}

void ProcessPrimitive(cgltf_primitive* GltfPrimitive, gpu_mesh* Mesh, hmm_mat4 Transform)
{
    gltf_primitive Primitive;
    Primitive.InstanceData.Transform = Transform;

    if (GltfPrimitive->type != cgltf_primitive_type_triangles)
        return;

    cgltf_attribute* PositionAttribute = NULL;
    cgltf_attribute* TexcoordAttribute = NULL;
    cgltf_attribute* NormalAttribute = NULL;

    for (i32 AttributeIndex = 0; AttributeIndex < GltfPrimitive->attributes_count; AttributeIndex++) {
        cgltf_attribute* Attribute = &GltfPrimitive->attributes[AttributeIndex];

        if (strcmp(Attribute->name, "POSITION") == 0) PositionAttribute = Attribute;
        if (strcmp(Attribute->name, "TEXCOORD_0") == 0) TexcoordAttribute = Attribute;
        if (strcmp(Attribute->name, "NORMAL") == 0) NormalAttribute = Attribute;
    }

    assert(PositionAttribute && TexcoordAttribute && NormalAttribute);

    u32 VertexCount = (u32)PositionAttribute->data->count;
    u64 VertexBufferSize = VertexCount * sizeof(mesh_vertex);
    std::vector<mesh_vertex> Vertices(VertexCount);

    CODE_BLOCK("Position")
    {
        u32 ComponentSize, ComponentCount;
        f32* Source = (f32*)GetAccessorData(PositionAttribute->data, &ComponentSize, &ComponentCount);
        assert(ComponentSize == 4);

        if (Source)
        {
            for (u32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
                Vertices[VertexIndex].Position.X = Source[VertexIndex * ComponentCount + 0];
                Vertices[VertexIndex].Position.Y = Source[VertexIndex * ComponentCount + 1];
                Vertices[VertexIndex].Position.Z = Source[VertexIndex * ComponentCount + 2];
            }
        }
    }

    CODE_BLOCK("Texcoords")
    {
        u32 ComponentSize, ComponentCount;
        f32* Source = (f32*)GetAccessorData(TexcoordAttribute->data, &ComponentSize, &ComponentCount);
        assert(ComponentSize == 4);
        if (Source)
        {
            for (u32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
                Vertices[VertexIndex].UV.X = Source[VertexIndex * ComponentCount + 0];
                Vertices[VertexIndex].UV.Y = Source[VertexIndex * ComponentCount + 1];
            }
        }
    }

    CODE_BLOCK("Normals")
    {
        u32 ComponentSize, ComponentCount;
        f32* Source = (f32*)GetAccessorData(NormalAttribute->data, &ComponentSize, &ComponentCount);
        assert(ComponentSize == 4);

        if (Source)
        {
            for (u32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
                Vertices[VertexIndex].Normals.X = Source[VertexIndex * ComponentCount + 0];
                Vertices[VertexIndex].Normals.Y = Source[VertexIndex * ComponentCount + 1];
                Vertices[VertexIndex].Normals.Z = Source[VertexIndex * ComponentCount + 2];
            }
        }
    }

    Primitive.IndexCount = (u32)GltfPrimitive->indices->count;
    u32 IndexBufferSize = Primitive.IndexCount * sizeof(u32);
    std::vector<u32> Indices(Primitive.IndexCount);

    CODE_BLOCK("Indices")
    {
        if (GltfPrimitive->indices != NULL) {
            for (u32 Index = 0; Index < (u32)GltfPrimitive->indices->count; Index++) {
                Indices[Index] = (u32)(cgltf_accessor_read_index(GltfPrimitive->indices, Index));
            }
        }
    }

    for (u32 TriangleIndex = 0; TriangleIndex < Primitive.IndexCount; TriangleIndex += 3) {
        hmm_vec3 Pos1 = Vertices[Indices[TriangleIndex + 0]].Position;
        hmm_vec3 Pos2 = Vertices[Indices[TriangleIndex + 1]].Position;
        hmm_vec3 Pos3 = Vertices[Indices[TriangleIndex + 2]].Position;

        hmm_vec2 UV1 = Vertices[Indices[TriangleIndex + 0]].UV;
        hmm_vec2 UV2 = Vertices[Indices[TriangleIndex + 1]].UV;
        hmm_vec2 UV3 = Vertices[Indices[TriangleIndex + 2]].UV;
        
        hmm_vec2 DeltaUV1 = HMM_SubtractVec2(UV2, UV1);
		hmm_vec2 DeltaUV2 = HMM_SubtractVec2(UV3, UV1);

		hmm_vec3 Edge1 = HMM_SubtractVec3(Pos2, Pos1);
		hmm_vec3 Edge2 = HMM_SubtractVec3(Pos3, Pos1);

        f32 F = 1.0f / (DeltaUV1.X * DeltaUV2.Y - DeltaUV2.X * DeltaUV1.Y);

        hmm_vec3 Tangent = HMM_Vec3(
            F * (DeltaUV2.Y * Edge1.X - DeltaUV1.Y * Edge2.X),
			F * (DeltaUV2.Y * Edge1.Y - DeltaUV1.Y * Edge2.Y),
			F * (DeltaUV2.Y * Edge1.Z - DeltaUV1.Y * Edge2.Z)
        );

        hmm_vec3 Bitangent = HMM_Vec3(
            F * (-DeltaUV2.Y * Edge1.X + DeltaUV1.Y * Edge2.X),
			F * (-DeltaUV2.Y * Edge1.Y + DeltaUV1.Y * Edge2.Y),
			F * (-DeltaUV2.Y * Edge1.Z + DeltaUV1.Y * Edge2.Z)
        );

        Vertices[Indices[TriangleIndex + 0]].Tangent = Tangent;
        Vertices[Indices[TriangleIndex + 1]].Tangent = Tangent;
        Vertices[Indices[TriangleIndex + 2]].Tangent = Tangent;

        Vertices[Indices[TriangleIndex + 0]].Bitangent = Bitangent;
        Vertices[Indices[TriangleIndex + 1]].Bitangent = Bitangent;
        Vertices[Indices[TriangleIndex + 2]].Bitangent = Bitangent;
    }

    CODE_BLOCK("AABB")
    {
        aabb BoundingBox;
        BoundingBox.Min = HMM_Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        BoundingBox.Max = HMM_Vec3(FLT_MIN, FLT_MIN, FLT_MIN);

        for (u32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
            const mesh_vertex* Vertex = &Vertices[VertexIndex];

            BoundingBox.Min.X = std::min(BoundingBox.Min.X, Vertex->Position.X);
            BoundingBox.Min.Y = std::min(BoundingBox.Min.Y, Vertex->Position.Y);
            BoundingBox.Min.Z = std::min(BoundingBox.Min.Z, Vertex->Position.Z);

            BoundingBox.Max.X = std::max(BoundingBox.Max.X, Vertex->Position.X);
            BoundingBox.Max.Y = std::max(BoundingBox.Max.Y, Vertex->Position.Y);
            BoundingBox.Max.Z = std::max(BoundingBox.Max.Z, Vertex->Position.Z);
        }

        hmm_vec3 Extent = HMM_MultiplyVec3f(HMM_SubtractVec3(BoundingBox.Max, BoundingBox.Min), 0.5f);
        hmm_vec3 Center = HMM_AddVec3(BoundingBox.Max, Extent);
        Primitive.InstanceData.BoundingSphere.XYZ = Center;

        for (u32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
            const mesh_vertex* Vertex = &Vertices[VertexIndex];

            Primitive.InstanceData.BoundingSphere.W = std::max(Primitive.InstanceData.BoundingSphere.W, HMM_DistanceVec3(Primitive.InstanceData.BoundingSphere.XYZ, Vertex->Position));
        }
    }

    BufferInit(&Primitive.VertexBuffer, VertexBufferSize, sizeof(mesh_vertex), BufferUsage_Vertex);
    BufferUpload(&Primitive.VertexBuffer, Vertices.data());

    BufferInit(&Primitive.IndexBuffer, IndexBufferSize, 0, BufferUsage_Index);
    BufferUpload(&Primitive.IndexBuffer, Indices.data());

    BufferInit(&Primitive.InstanceBuffer, sizeof(instance_data), 0, BufferUsage_Uniform);
    BufferUpload(&Primitive.InstanceBuffer, &Primitive.InstanceData);

    CODE_BLOCK("Material loading")
    {
        if (GltfPrimitive->material)
        {
            Primitive.MaterialIndex = (u32)Mesh->Materials.size();

            gltf_material Material = {};

            CODE_BLOCK("Albedo")
            {
                std::string AlbedoPath = Mesh->Directory + std::string(GltfPrimitive->material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
                TextureLoad(&Material.Albedo, AlbedoPath.c_str());

                Material.MaterialData.AlbedoFactor = HMM_Vec3(GltfPrimitive->material->pbr_metallic_roughness.base_color_factor[0], GltfPrimitive->material->pbr_metallic_roughness.base_color_factor[1], GltfPrimitive->material->pbr_metallic_roughness.base_color_factor[2]);
            }

            CODE_BLOCK("Normal")
            {
                if (GltfPrimitive->material->normal_texture.texture) {
                    Material.HasNormalMap = true;
                    std::string NormalPath = Mesh->Directory + std::string(GltfPrimitive->material->normal_texture.texture->image->uri);
                    TextureLoad(&Material.Normal, NormalPath.c_str());
                }
            }

            CODE_BLOCK("Metallic Roughness")
            {
                if (GltfPrimitive->material->pbr_metallic_roughness.metallic_roughness_texture.texture) {
                    Material.HasPBRMap = true;
                    std::string PBRPath = Mesh->Directory + std::string(GltfPrimitive->material->pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri);
                    TextureLoad(&Material.PBR, PBRPath.c_str());

                    Material.MaterialData.MetallicFactor = GltfPrimitive->material->pbr_metallic_roughness.metallic_factor;
                    Material.MaterialData.RoughnessFactor = GltfPrimitive->material->pbr_metallic_roughness.roughness_factor;
                }
            }

            BufferInit(&Material.MaterialBuffer, sizeof(material_data), 0, BufferUsage_Uniform);
            BufferUpload(&Material.MaterialBuffer, &Material.MaterialData);

            Mesh->Materials.push_back(Material);
        }
    }

    Primitive.InstanceData.PrimitiveIndex = (u32)Mesh->Primitives.size();
    Primitive.VertexCount = VertexCount;
    Primitive.TriangleCount = Primitive.IndexCount / 3;
    Primitive.VertexBufferSize = VertexBufferSize;
    Primitive.IndexBufferSize = IndexBufferSize;

    Mesh->TotalVertexCount += Primitive.VertexCount;
    Mesh->TotalIndexCount += Primitive.IndexCount;
    Mesh->TotalTriangleCount += Primitive.TriangleCount;

    Mesh->Primitives.push_back(Primitive);
}

void ProcessNode(cgltf_node* Node, gpu_mesh* Mesh)
{
    if (Node->mesh)
    {
        hmm_mat4 Transform = HMM_Mat4d(1.0f);
        if (Node->has_translation) {
            hmm_vec3 Translation = HMM_Vec3(Node->translation[0], Node->translation[1], Node->translation[2]);
            Transform = HMM_MultiplyMat4(Transform, HMM_Translate(Translation));
        }
        if (Node->has_rotation) {
            hmm_quaternion Rotation = HMM_Quaternion(Node->rotation[0], Node->rotation[1], Node->rotation[2], Node->rotation[3]);
            Transform = HMM_MultiplyMat4(Transform, HMM_QuaternionToMat4(Rotation));
        }
        if (Node->has_scale) {
            hmm_vec3 Scale = HMM_Vec3(Node->scale[0], Node->scale[1], Node->scale[2]);
            Transform = HMM_MultiplyMat4(Transform, HMM_Scale(Scale));
        }

        for (i32 GltfPrimitiveIndex = 0; GltfPrimitiveIndex < Node->mesh->primitives_count; GltfPrimitiveIndex++) {
            ProcessPrimitive(&Node->mesh->primitives[GltfPrimitiveIndex], Mesh, Transform);
        }
    }

    for (i32 ChildrenIndex = 0; ChildrenIndex < Node->children_count; ChildrenIndex++) {
        ProcessNode(Node->children[ChildrenIndex], Mesh);
    }
}

void GpuMeshLoad(gpu_mesh* Mesh, const std::string& Path)
{
    cgltf_options Options;
    memset(&Options, 0, sizeof(Options));
    cgltf_data* Data = NULL;
    
    CGLTFCall(cgltf_parse_file(&Options, Path.c_str(), &Data));
    CGLTFCall(cgltf_load_buffers(&Options, Data, Path.c_str()));
    cgltf_scene* Scene = Data->scene;

    size_t Position = Path.find_last_of('/');
    std::string Directory = Path.substr(0, Position + 1);
    Mesh->Directory = Directory;

    for (i32 NodeIndex = 0; NodeIndex < Scene->nodes_count; NodeIndex++)
        ProcessNode(Scene->nodes[NodeIndex], Mesh);

    cgltf_free(Data);
}

void GpuMeshFree(gpu_mesh* Mesh)
{
    for (gltf_material Material : Mesh->Materials) {
        BufferFree(&Material.MaterialBuffer);
        if (Material.Albedo.Internal) {
            TextureFree(&Material.Albedo);
        }
        if (Material.HasNormalMap) {
            TextureFree(&Material.Normal);
        }
        if (Material.HasPBRMap) {
            TextureFree(&Material.PBR);
        }
    }

    for (gltf_primitive Primitive : Mesh->Primitives) {
        BufferFree(&Primitive.IndexBuffer);
        BufferFree(&Primitive.VertexBuffer);
    }
}