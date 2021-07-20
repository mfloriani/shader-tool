#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <string>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

namespace D3DUtil
{
    class DxException
    {
    public:
        DxException() = default;
        DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

        std::wstring ToString()const;

        HRESULT ErrorCode = S_OK;
        std::wstring FunctionName;
        std::wstring Filename;
        int LineNumber = -1;
    };

    inline std::wstring AnsiToWString(const std::string& str)
    {
        WCHAR buffer[512];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
        return std::wstring(buffer);
    }

    #ifndef ThrowIfFailed
    #define ThrowIfFailed(x)                                              \
    {                                                                     \
        HRESULT hr__ = (x);                                               \
        std::wstring wfn = D3DUtil::AnsiToWString(__FILE__);                       \
        if(FAILED(hr__)) { throw D3DUtil::DxException(hr__, L#x, wfn, __LINE__); } \
    }
    #endif

	UINT CalcConstantBufferByteSize(UINT byteSize);

	DirectX::XMFLOAT4X4 Identity4x4();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
	
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	struct SHADER_DESC
	{
		SHADER_DESC() {}
		SHADER_DESC(D3D12_SHADER_DESC desc)
		{
			Version = desc.Version;
			Creator = desc.Creator;
			Flags = desc.Flags;

			ConstantBuffers = desc.ConstantBuffers;
			BoundResources = desc.BoundResources;
			InputParameters = desc.InputParameters;
			OutputParameters = desc.OutputParameters;

			InstructionCount = desc.InstructionCount;
			TempRegisterCount = desc.TempRegisterCount;
			TempArrayCount = desc.TempArrayCount;
			DefCount = desc.DefCount;
			DclCount = desc.DclCount;
			TextureNormalInstructions = desc.TextureNormalInstructions;
			TextureLoadInstructions = desc.TextureLoadInstructions;
			TextureCompInstructions = desc.TextureCompInstructions;
			TextureBiasInstructions = desc.TextureBiasInstructions;
			TextureGradientInstructions = desc.TextureGradientInstructions;
			FloatInstructionCount = desc.FloatInstructionCount;
			IntInstructionCount = desc.IntInstructionCount;
			UintInstructionCount = desc.UintInstructionCount;
			StaticFlowControlCount = desc.StaticFlowControlCount;
			DynamicFlowControlCount = desc.DynamicFlowControlCount;
			MacroInstructionCount = desc.MacroInstructionCount;
			ArrayInstructionCount = desc.ArrayInstructionCount;
			CutInstructionCount = desc.CutInstructionCount;
			EmitInstructionCount = desc.EmitInstructionCount;
			GSOutputTopology = desc.GSOutputTopology;
			GSMaxOutputVertexCount = desc.GSMaxOutputVertexCount;
			InputPrimitive = desc.InputPrimitive;
			PatchConstantParameters = desc.PatchConstantParameters;
			cGSInstanceCount = desc.cGSInstanceCount;
			cControlPoints = desc.cControlPoints;
			HSOutputPrimitive = desc.HSOutputPrimitive;
			HSPartitioning = desc.HSPartitioning;
			TessellatorDomain = desc.TessellatorDomain;

			cBarrierInstructions = desc.cBarrierInstructions;
			cInterlockedInstructions = desc.cInterlockedInstructions;
			cTextureStoreInstructions = desc.cTextureStoreInstructions;
		}

		UINT                    Version;                     // Shader version
		std::string             Creator;                     // Creator string
		UINT                    Flags;                       // Shader compilation/parse flags

		UINT                    ConstantBuffers;             // Number of constant buffers
		UINT                    BoundResources;              // Number of bound resources
		UINT                    InputParameters;             // Number of parameters in the input signature
		UINT                    OutputParameters;            // Number of parameters in the output signature

		UINT                    InstructionCount;            // Number of emitted instructions
		UINT                    TempRegisterCount;           // Number of temporary registers used 
		UINT                    TempArrayCount;              // Number of temporary arrays used
		UINT                    DefCount;                    // Number of constant defines 
		UINT                    DclCount;                    // Number of declarations (input + output)
		UINT                    TextureNormalInstructions;   // Number of non-categorized texture instructions
		UINT                    TextureLoadInstructions;     // Number of texture load instructions
		UINT                    TextureCompInstructions;     // Number of texture comparison instructions
		UINT                    TextureBiasInstructions;     // Number of texture bias instructions
		UINT                    TextureGradientInstructions; // Number of texture gradient instructions
		UINT                    FloatInstructionCount;       // Number of floating point arithmetic instructions used
		UINT                    IntInstructionCount;         // Number of signed integer arithmetic instructions used
		UINT                    UintInstructionCount;        // Number of unsigned integer arithmetic instructions used
		UINT                    StaticFlowControlCount;      // Number of static flow control instructions used
		UINT                    DynamicFlowControlCount;     // Number of dynamic flow control instructions used
		UINT                    MacroInstructionCount;       // Number of macro instructions used
		UINT                    ArrayInstructionCount;       // Number of array instructions used
		UINT                    CutInstructionCount;         // Number of cut instructions used
		UINT                    EmitInstructionCount;        // Number of emit instructions used
		D3D_PRIMITIVE_TOPOLOGY  GSOutputTopology;            // Geometry shader output topology
		UINT                    GSMaxOutputVertexCount;      // Geometry shader maximum output vertex count
		D3D_PRIMITIVE           InputPrimitive;              // GS/HS input primitive
		UINT                    PatchConstantParameters;     // Number of parameters in the patch constant signature
		UINT                    cGSInstanceCount;            // Number of Geometry shader instances
		UINT                    cControlPoints;              // Number of control points in the HS->DS stage
		D3D_TESSELLATOR_OUTPUT_PRIMITIVE HSOutputPrimitive;  // Primitive output by the tessellator
		D3D_TESSELLATOR_PARTITIONING HSPartitioning;         // Partitioning mode of the tessellator
		D3D_TESSELLATOR_DOMAIN  TessellatorDomain;           // Domain of the tessellator (quad, tri, isoline)
		// instruction counts
		UINT cBarrierInstructions;                           // Number of barrier instructions in a compute shader
		UINT cInterlockedInstructions;                       // Number of interlocked instructions
		UINT cTextureStoreInstructions;                      // Number of texture writes
	};

	struct SHADER_BUFFER_DESC // D3D12_SHADER_BUFFER_DESC
	{
		SHADER_BUFFER_DESC() {}
		SHADER_BUFFER_DESC(D3D12_SHADER_BUFFER_DESC bufferDesc)
		{
			Name = bufferDesc.Name;
			Type = bufferDesc.Type;
			Variables = bufferDesc.Variables;
			Size = bufferDesc.Size;
			uFlags = bufferDesc.uFlags;
		}

		std::string             Name;           // Name of the constant buffer
		D3D_CBUFFER_TYPE        Type;           // Indicates type of buffer content
		UINT                    Variables;      // Number of member variables
		UINT                    Size;           // Size of CB (in bytes)
		UINT                    uFlags;         // Buffer description flags
	};

	struct SHADER_TYPE_DESC // D3D12_SHADER_TYPE_DESC
	{
		SHADER_TYPE_DESC() {}
		SHADER_TYPE_DESC(D3D12_SHADER_TYPE_DESC typeDesc)
		{
			Class = typeDesc.Class;
			Type = typeDesc.Type;
			Rows = typeDesc.Rows;
			Columns = typeDesc.Columns;
			Elements = typeDesc.Elements;
			Members = typeDesc.Members;
			Offset = typeDesc.Offset;
			Name = typeDesc.Name;
		}

		D3D_SHADER_VARIABLE_CLASS   Class;          // Variable class (e.g. object, matrix, etc.)
		D3D_SHADER_VARIABLE_TYPE    Type;           // Variable type (e.g. float, sampler, etc.)
		UINT                        Rows;           // Number of rows (for matrices, 1 for other numeric, 0 if not applicable)
		UINT                        Columns;        // Number of columns (for vectors & matrices, 1 for other numeric, 0 if not applicable)
		UINT                        Elements;       // Number of elements (0 if not an array)
		UINT                        Members;        // Number of members (0 if not a structure)
		UINT                        Offset;         // Offset from the start of structure (0 if not a structure member)
		std::string                 Name;           // Name of type
	};

	struct SHADER_VARIABLE_DESC // D3D12_SHADER_VARIABLE_DESC
	{
		SHADER_VARIABLE_DESC() {}
		SHADER_VARIABLE_DESC(D3D12_SHADER_VARIABLE_DESC varDesc, D3D12_SHADER_TYPE_DESC typeDesc)
		{
			Name = varDesc.Name;
			StartOffset = varDesc.StartOffset;
			Size = varDesc.Size;
			uFlags = varDesc.uFlags;
			DefaultValue = varDesc.DefaultValue;
			StartTexture = varDesc.StartTexture;
			TextureSize = varDesc.TextureSize;
			StartSampler = varDesc.StartSampler;
			SamplerSize = varDesc.SamplerSize;
			Type = SHADER_TYPE_DESC(typeDesc);
		}

		std::string             Name;           // Name of the variable
		UINT                    StartOffset;    // Offset in constant buffer's backing store
		UINT                    Size;           // Size of variable (in bytes)
		UINT                    uFlags;         // Variable flags
		LPVOID                  DefaultValue;   // Raw pointer to default value
		UINT                    StartTexture;   // First texture index (or -1 if no textures used)
		UINT                    TextureSize;    // Number of texture slots possibly used.
		UINT                    StartSampler;   // First sampler index (or -1 if no textures used)
		UINT                    SamplerSize;    // Number of sampler slots possibly used.
		SHADER_TYPE_DESC        Type;           // Type struct of the variable
	};
}
