#include "hzpch.h"
#include "Shader.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/IO/FileSystem.hpp"

#include "Horizon/Vulkan/VulkanShader.hpp"

#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

#include <streambuf>

namespace Hz
{

    static shaderc_shader_kind ShaderStageToShaderCType(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:                       return shaderc_glsl_vertex_shader;
		case ShaderStage::Fragment:                     return shaderc_glsl_fragment_shader;
		case ShaderStage::Compute:                      return shaderc_glsl_compute_shader;
        case ShaderStage::Geometry:                     return shaderc_glsl_geometry_shader;
        case ShaderStage::TessellationControl:          return shaderc_glsl_tess_control_shader;
        case ShaderStage::TessellationEvaluation:       return shaderc_glsl_tess_evaluation_shader;

        case ShaderStage::RayGenKHR: /*Also NV*/        return shaderc_glsl_raygen_shader;
        case ShaderStage::AnyHitKHR: /*Also NV*/        return shaderc_glsl_anyhit_shader;
        case ShaderStage::ClosestHitKHR: /*Also NV*/    return shaderc_glsl_closesthit_shader;
        case ShaderStage::MissKHR: /*Also NV*/          return shaderc_glsl_miss_shader;
        case ShaderStage::IntersectionKHR: /*Also NV*/  return shaderc_glsl_intersection_shader;
        case ShaderStage::CallableKHR: /*Also NV*/      return shaderc_glsl_callable_shader;

        case ShaderStage::TaskEXT: /*Also NV*/          return shaderc_glsl_task_shader;
        case ShaderStage::MeshEXT: /*Also NV*/          return shaderc_glsl_mesh_shader;

		default:
			HZ_LOG_ERROR("ShaderStage passed in is currently not supported.");
			break;
		}

		// Return vertex shader by default.
		return shaderc_glsl_vertex_shader;
	}

    std::vector<char> ShaderCompiler::CompileGLSL(ShaderStage stage, const std::string& code)
    {
        HZ_VERIFY((!code.empty()), "Empty string passed in as shader code.");

        shaderc::Compiler compiler = {};
		shaderc::CompileOptions options = {};
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(code, ShaderStageToShaderCType(stage), "", options);

		HZ_ASSERT((module.GetCompilationStatus() == shaderc_compilation_status_success), "Error compiling shader: {0}", module.GetErrorMessage());

		// Convert SPIR-V code to vector<char>
		const uint32_t* data = module.cbegin();
		const size_t numWords = module.cend() - module.cbegin();
		const size_t sizeInBytes = numWords * sizeof(uint32_t);
		const char* bytes = reinterpret_cast<const char*>(data);

		return std::vector<char>(bytes, bytes + sizeInBytes);
    }

    Ref<Shader> Shader::Create(const ShaderSpecification& specs)
    {
        if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
            return Ref<VulkanShader>::Create(specs);

        return nullptr;
    }

    std::string Shader::ReadGLSL(const std::filesystem::path& path)
    {
		IO::IFile file(path);

        std::string content = file.ReadAll();
        if (content.empty())
            HZ_LOG_WARN("GLSL file: '{0}' is empty, this will 'cause internal errors.", path.string());

		return content;
    }

    std::vector<char> Shader::ReadSPIRV(const std::filesystem::path& path)
    {
		IO::IFile file(path, IO::FileMode::Ate | IO::FileMode::Binary);

		size_t fileSize = file.Tell();
		std::vector<char> buffer(fileSize);

		file.Seek(0);
		file.Read(buffer, fileSize);

		return buffer;
    }

}
