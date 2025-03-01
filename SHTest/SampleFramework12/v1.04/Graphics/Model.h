//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "..\\PCH.h"

#include "..\\InterfacePointers.h"
#include "..\\SF12_Math.h"
#include "..\\Serialization.h"
#include "..\\Containers.h"
#include "GraphicsTypes.h"
#include "..\\Shaders\Mesh_Shared.h"

struct aiMesh;

namespace SampleFramework12
{

enum class MaterialTextures
{
    Albedo = 0,
    Normal,
    Roughness,
    Metallic,
    Opacity,
    Emissive,

    Count
};

struct MeshMaterial
{
    std::string Name;
    std::wstring TextureNames[uint64(MaterialTextures::Count)];
    const Texture* Textures[uint64(MaterialTextures::Count)] = { };
    uint32 TextureIndices[uint64(MaterialTextures::Count)] = { };
    bool32 Opaque = true;

    uint32 Texture(MaterialTextures texType) const
    {
        Assert_(uint64(texType) < uint64(MaterialTextures::Count));
        Assert_(Textures[uint64(texType)] != nullptr);
        return Textures[uint64(texType)]->SRV;
    }

    template<typename TSerializer> void Serialize(TSerializer& serializer)
    {
        for(uint64 i = 0; i < uint64(MaterialTextures::Count); ++i)
            SerializeItem(serializer, TextureNames[i]);
        BulkSerializeArray(serializer, TextureIndices, ArraySize_(TextureIndices));
        SerializeItem(serializer, Opaque);
    }
};

struct MeshPart
{
    uint32 VertexStart;
    uint32 VertexCount;
    uint32 IndexStart;
    uint32 IndexCount;
    uint32 MaterialIdx;

    MeshPart() : VertexStart(0), VertexCount(0), IndexStart(0), IndexCount(0), MaterialIdx(0)
    {
    }
};

enum class IndexType
{
    Index16Bit = 0,
    Index32Bit = 1
};

enum class InputElementType : uint64
{
    Position = 0,
    Normal,
    Tangent,
    Bitangent,
    UV,

    NumTypes,
};

struct MaterialTexture
{
    std::wstring Name;
    Texture Texture;
};

struct ModelSpotLight
{
    Float3 Position;
    Float3 Intensity;
    Float3 Direction;
    Quaternion Orientation;
    Float2 AngularAttenuation;
};

struct ModelPointLight
{
    Float3 Position;
    Float3 Intensity;
};

struct ModelLoadSettings;

class Mesh
{
    friend class Model;

public:

    ~Mesh()
    {
        Assert_(numVertices == 0);
    }

    // Init from loaded files
    void InitFromAssimpMesh(const aiMesh& assimpMesh, const ModelLoadSettings& loadSettings,
                            MeshVertex* dstVertices, uint8* dstIndices, IndexType indexType,
                            const Float4x4& transform);

    // Procedural generation
    void InitBox(const Float3& dimensions, const Float3& position,
                 const Quaternion& orientation, uint32 materialIdx,
                 MeshVertex* dstVertices, uint16* dstIndices);

    void InitPlane(const Float2& dimensions, const Float3& position,
                   const Quaternion& orientation, uint32 materialIdx,
                   MeshVertex* dstVertices, uint16* dstIndices);

    void InitCommon(const MeshVertex* vertices, const uint8* indices, uint64 vbAddress, uint64 ibAddress, uint64 vtxOffset, uint64 idxOffset);

    void Shutdown();

    // Accessors
    const Array<MeshPart>& MeshParts() const { return meshParts; }
    uint64 NumMeshParts() const { return meshParts.Size(); }

    uint32 NumVertices() const { return numVertices; }
    uint32 NumIndices() const { return numIndices; }
    uint32 VertexOffset() const { return vtxOffset; }
    uint32 IndexOffset() const { return idxOffset; }

    IndexType IndexBufferType() const { return indexType; }
    DXGI_FORMAT IndexBufferFormat() const { return indexType == IndexType::Index32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT; }
    uint32 IndexSize() const { return indexType == IndexType::Index32Bit ? 4 : 2; }

    const MeshVertex* Vertices() const { return vertices; }
    const uint16* Indices() const { Assert_(indexType == IndexType::Index16Bit); return (const uint16*)indices; }
    const uint32* Indices32() const { Assert_(indexType == IndexType::Index32Bit); return (const uint32*)indices; }

    const uint32 Index(uint32 idx) const
    {
        Assert_(idx < numIndices);
        if(indexType == IndexType::Index16Bit)
            return reinterpret_cast<const uint16*>(indices)[idx];
        else
            return reinterpret_cast<const uint32*>(indices)[idx];
    }

    const D3D12_VERTEX_BUFFER_VIEW* VBView() const { return &vbView; }
    const D3D12_INDEX_BUFFER_VIEW* IBView() const { return &ibView; }

    const Float3& AABBMin() const { return aabbMin; }
    const Float3& AABBMax() const { return aabbMax; }

    uint32 MeshletOffset() const { return meshletOffset; }
    uint32 NumMeshlets() const { return numMeshlets; }

    static const char* InputElementTypeString(InputElementType elemType);

    template<typename TSerializer> void Serialize(TSerializer& serializer)
    {
        BulkSerializeItem(serializer, meshParts);
        SerializeItem(serializer, numVertices);
        SerializeItem(serializer, numIndices);
        SerializeItem(serializer, vtxOffset);
        SerializeItem(serializer, idxOffset);
        uint32 idxType = uint32(indexType);
        SerializeItem(serializer, idxType);
        indexType = IndexType(idxType);
        SerializeItem(serializer, aabbMin);
        SerializeItem(serializer, aabbMax);
        SerializeItem(serializer, meshletOffset);
        SerializeItem(serializer, numMeshlets);
    }

protected:

    Array<MeshPart> meshParts;

    uint32 numVertices = 0;
    uint32 numIndices = 0;
    uint32 vtxOffset = 0;
    uint32 idxOffset = 0;

    IndexType indexType = IndexType::Index16Bit;

    const MeshVertex* vertices = nullptr;
    const uint8* indices = nullptr;

    D3D12_VERTEX_BUFFER_VIEW vbView = { };
    D3D12_INDEX_BUFFER_VIEW ibView = { };

    Float3 aabbMin;
    Float3 aabbMax;

    uint32 meshletOffset = 0;
    uint32 numMeshlets = 0;
};

struct ModelLoadSettings
{
    const wchar* FilePath = nullptr;
    const wchar* TextureDir = nullptr;
    float SceneScale = 1.0f;
    bool ForceSRGB = false;
    bool MergeMeshes = true;
    bool ConvertFromZUp = false;
    bool GenerateMeshlets = false;
};

struct ProceduralModelInit
{
    const MeshVertex* Vertices = nullptr;
    const uint32* Indices = nullptr;
    uint32 NumVertices = 0;
    uint32 NumIndices = 0;
    const wchar* TexturePaths[uint64(MaterialTextures::Count)] = { };
    bool ForceSRGB = false;
    bool GenerateMeshlets = false;
};

struct BoxSceneInit
{
    Float3 Dimensions = Float3(1.0f, 1.0f, 1.0f);
    Float3 Position = Float3();
    Quaternion Orientation = Quaternion();
    const wchar* ColorMap = L"";
    const wchar* NormalMap = L"";
    bool GenerateMeshlets = false;
};

struct BoxTestSceneInit
{
    Float3 BottomBoxDimensions = Float3(10.0f, 0.25f, 10.0f);
    Float3 BottomBoxPosition = Float3(0.0f);
    Float3 TopBoxDimensions = Float3(2.0f);
    Float3 TopBoxPosition = Float3(0.0f, 1.5f, 0.0f);
    bool GenerateMeshlets = false;
};

struct PlaneSceneInit
{
    Float2 Dimensions = Float2(1.0f, 1.0f);
    Float3 Position = Float3();
    Quaternion Orientation = Quaternion();
    const wchar* ColorMap = L"";
    const wchar* NormalMap = L"";
    bool GenerateMeshlets = false;
};

class Model
{
public:

    ~Model()
    {
        Assert_(meshes.Size() == 0);
    }

    // Loading from file formats
    void CreateWithAssimp(const ModelLoadSettings& settings);

    void CreateFromMeshData(const wchar* filePath);

    // Procedural generation
    void GenerateBoxScene(const BoxSceneInit& init);
    void GenerateBoxTestScene(const BoxTestSceneInit& init);
    void GeneratePlaneScene(const PlaneSceneInit& init);

    void CreateProcedural(const ProceduralModelInit& init);

    void Shutdown();

    // Accessors
    const Array<Mesh>& Meshes() const { return meshes; }
    uint64 NumMeshes() const { return meshes.Size(); }

    const Float3& AABBMin() const { return aabbMin; }
    const Float3& AABBMax() const { return aabbMax; }

    const Array<MeshMaterial>& Materials() const { return meshMaterials; }
    const List<MaterialTexture*>& MaterialTextures() const { return materialTextures; }

    const Array<ModelSpotLight>& SpotLights() const { return spotLights; }
    const Array<ModelPointLight>& PointLights() const { return pointLights; }

    const StructuredBuffer& VertexBuffer() const { return vertexBuffer; }
    const FormattedBuffer& IndexBuffer() const { return indexBuffer; }

    const List<Meshlet>& Meshlets() const { return meshlets; }
    const List<uint32>& MeshletVertices() const { return meshletVertices; }
    const List<MeshletTriangle>& MeshletTriangles() const { return meshletTriangles; }

    const StructuredBuffer& MeshletBuffer() const { return meshletBuffer; }
    const RawBuffer& MeshletVerticesBuffer() const { return meshletVerticesBuffer; }
    const StructuredBuffer& MeshletTrianglesBuffer() const { return meshletTrianglesBuffer; }
    const StructuredBuffer& MeshletBoundsBuffer() const { return meshletBoundsBuffer; }

    const MeshVertex* Vertices() const { return vertices.Data(); }
    const uint16* Indices() const { Assert_(indexType == IndexType::Index16Bit); return (const uint16*)indices.Data(); }
    const uint32* Indices32() const { Assert_(indexType == IndexType::Index32Bit); return (const uint32*)indices.Data(); }

    static const D3D12_INPUT_ELEMENT_DESC* InputElements();
    static const InputElementType* InputElementTypes();
    static uint64 NumInputElements();

    IndexType IndexBufferType() const { return indexType; }
    DXGI_FORMAT IndexBufferFormat() const { return indexType == IndexType::Index32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT; }
    uint32 IndexSize() const { return indexType == IndexType::Index32Bit ? 4 : 2; }

    // Serialization
    template<typename TSerializer>
    void Serialize(TSerializer& serializer)
    {
        SerializeItem(serializer, meshes);
        SerializeItem(serializer, meshMaterials);
        BulkSerializeItem(serializer, spotLights);
        BulkSerializeItem(serializer, pointLights);
        SerializeItem(serializer, textureDirectory);
        SerializeItem(serializer, forceSRGB);
        SerializeItem(serializer, aabbMin);
        SerializeItem(serializer, aabbMax);
        BulkSerializeItem(serializer, vertices);
        BulkSerializeItem(serializer, indices);
        uint32 idxType = uint32(indexType);
        SerializeItem(serializer, idxType);
        indexType = IndexType(idxType);
        BulkSerializeItem(serializer, meshlets);
        BulkSerializeItem(serializer, meshletVertices);
        BulkSerializeItem(serializer, meshletTriangles);
        BulkSerializeItem(serializer, meshletBounds);
    }

protected:

    void GenerateMeshlets();
    void CreateBuffers();

    Array<Mesh> meshes;
    Array<MeshMaterial> meshMaterials;
    Array<ModelSpotLight> spotLights;
    Array<ModelPointLight> pointLights;
    std::wstring textureDirectory;
    bool32 forceSRGB = false;
    Float3 aabbMin;
    Float3 aabbMax;

    StructuredBuffer vertexBuffer;
    FormattedBuffer indexBuffer;
    Array<MeshVertex> vertices;
    Array<uint8> indices;
    IndexType indexType = IndexType::Index16Bit;

    List<Meshlet> meshlets;
    List<uint32> meshletVertices;
    List<MeshletTriangle> meshletTriangles;
    List<MeshletBounds> meshletBounds;

    StructuredBuffer meshletBuffer;
    RawBuffer meshletVerticesBuffer;
    StructuredBuffer meshletTrianglesBuffer;
    StructuredBuffer meshletBoundsBuffer;

    List<MaterialTexture*> materialTextures;
};

void MakeSphereGeometry(uint64 uDivisions, uint64 vDivisions, StructuredBuffer& vtxBuffer, FormattedBuffer& idxBuffer);
void MakeBoxGeometry(StructuredBuffer& vtxBuffer, FormattedBuffer& idxBuffer, float scale = 1.0f);
void MakeConeGeometry(uint64 divisions, StructuredBuffer& vtxBuffer, FormattedBuffer& idxBuffer, Array<Float3>& positions);
void MakeConeGeometry(uint64 divisions, StructuredBuffer& vtxBuffer, FormattedBuffer& idxBuffer);

}