#include <shaderc/shaderc.h>

#include <shaderc/shaderc.hpp>
#include <vector>

#include "base/base.hpp"

// NOTE: Panics if compilation fails.
std::vector<nbody::U32> compile_glsl(const std::string& name, shaderc_shader_kind kind,
                                     const std::string& source) {
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult result =
        compiler.CompileGlslToSpv(source, kind, name.c_str(), options);

    ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success,
           "Compilation failed");

    return {result.begin(), result.end()};
}

int main() { return 0; }
