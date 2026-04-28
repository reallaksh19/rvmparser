#include "RvmParserDll.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Parser.h"
#include "Store.h"
#include "Common.h"
#include "Tessellator.h"
#include "Colorizer.h"
#include "AddGroupBBox.h"

namespace {

thread_local std::string g_last_log;

void dllLogger(unsigned level, const char* msg, ...)
{
  char line[2048] = {};
  va_list args;
  va_start(args, msg);
#ifdef _WIN32
  vsnprintf_s(line, sizeof(line), _TRUNCATE, msg, args);
#else
  vsnprintf(line, sizeof(line), msg, args);
#endif
  va_end(args);

  switch (level) {
  case 0: g_last_log += "[I] "; break;
  case 1: g_last_log += "[W] "; break;
  default: g_last_log += "[E] "; break;
  }
  g_last_log += line;
  g_last_log += "\n";
}

void setError(char* errorBuffer, int errorBufferSize, const std::string& message)
{
  if (!errorBuffer || errorBufferSize <= 0) return;
#ifdef _WIN32
  strncpy_s(errorBuffer, static_cast<size_t>(errorBufferSize), message.c_str(), _TRUNCATE);
#else
  std::snprintf(errorBuffer, static_cast<size_t>(errorBufferSize), "%s", message.c_str());
#endif
}

std::vector<unsigned char> readBinaryFile(const char* path)
{
  if (!path || !*path) {
    throw std::runtime_error("Missing input file path.");
  }

  std::ifstream input(path, std::ios::binary);
  if (!input) {
    throw std::runtime_error(std::string("Failed to open input file: ") + path);
  }

  input.seekg(0, std::ios::end);
  const std::streamoff size = input.tellg();
  if (size < 0) {
    throw std::runtime_error(std::string("Failed to determine input file size: ") + path);
  }
  input.seekg(0, std::ios::beg);

  std::vector<unsigned char> data(static_cast<size_t>(size));
  if (!data.empty()) {
    input.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!input) {
      throw std::runtime_error(std::string("Failed to read complete input file: ") + path);
    }
  }

  return data;
}

bool hasText(const char* s)
{
  return s && *s;
}

struct ConvertOptions
{
  float tolerance = 0.1f;
  bool rotateZToY = true;
  bool centerModel = true;
  bool includeAttributes = true;
  bool mergeGeometries = true;
};

int runWithStore(
  const char* inputRvmPath,
  const char* inputAttPath,
  const ConvertOptions& options,
  const char* outputGlbPath,
  const char* outputJsonPath,
  const char* outputRevPath,
  char* errorBuffer,
  int errorBufferSize)
{
  g_last_log.clear();

  try {
    if (!hasText(inputRvmPath)) {
      throw std::runtime_error("input_rvm_path is required.");
    }

    if (!hasText(outputGlbPath) && !hasText(outputJsonPath) && !hasText(outputRevPath)) {
      throw std::runtime_error("At least one output path is required.");
    }

    Store store;

    const auto rvmBytes = readBinaryFile(inputRvmPath);
    if (!parseRVM(&store, dllLogger, inputRvmPath, rvmBytes.data(), rvmBytes.size())) {
      throw std::runtime_error(std::string("Failed to parse RVM: ") + store.errorString());
    }

    if (hasText(inputAttPath)) {
      const auto attBytes = readBinaryFile(inputAttPath);
      if (!parseAtt(&store, dllLogger, attBytes.data(), attBytes.size())) {
        throw std::runtime_error("Failed to parse ATT/TXT attribute file.");
      }
    }

    // Match CLI behavior: connect and align are always run before export.
    connect(&store, dllLogger);
    align(&store, dllLogger);

    const bool needsBBoxes = hasText(outputJsonPath) || hasText(outputGlbPath);
    if (needsBBoxes) {
      AddGroupBBox addGroupBBox;
      store.apply(&addGroupBBox);
    }

    if (hasText(outputGlbPath)) {
      Colorizer colorizer(dllLogger, nullptr);
      store.apply(&colorizer);

      const float tolerance = std::max(1e-6f, options.tolerance);
      const float cullLeafThreshold = -1.0f;
      const float cullGeometryThreshold = -1.0f;
      const unsigned maxSamples = 100;

      Tessellator tessellator(dllLogger, tolerance, cullLeafThreshold, cullGeometryThreshold, maxSamples);
      store.apply(&tessellator);

      if (!exportGLTF(&store,
                      dllLogger,
                      outputGlbPath,
                      0,
                      options.rotateZToY,
                      options.centerModel,
                      options.includeAttributes,
                      options.mergeGeometries)) {
        throw std::runtime_error(std::string("Failed to export GLB/GLTF: ") + outputGlbPath);
      }
    }

    if (hasText(outputJsonPath)) {
      if (!exportJson(&store, dllLogger, outputJsonPath)) {
        throw std::runtime_error(std::string("Failed to export JSON: ") + outputJsonPath);
      }
    }

    if (hasText(outputRevPath)) {
      if (!exportRev(&store, dllLogger, outputRevPath)) {
        throw std::runtime_error(std::string("Failed to export REV: ") + outputRevPath);
      }
    }

    setError(errorBuffer, errorBufferSize, g_last_log);
    return 0;
  }
  catch (const std::exception& ex) {
    std::string msg = ex.what();
    if (!g_last_log.empty()) {
      msg += "\n\nNative parser log:\n";
      msg += g_last_log;
    }
    setError(errorBuffer, errorBufferSize, msg);
    return 1;
  }
  catch (...) {
    std::string msg = "Unknown native RVM parser DLL error.";
    if (!g_last_log.empty()) {
      msg += "\n\nNative parser log:\n";
      msg += g_last_log;
    }
    setError(errorBuffer, errorBufferSize, msg);
    return 2;
  }
}

} // namespace

extern "C" {

RVMPARSER_DLL_API const char* RVMPARSER_DLL_CALL rvmparser_dll_version()
{
  return "rvmparser-dll/1.0";
}

RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_glb_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_glb_path,
    const char* output_json_path,
    char* error_buffer,
    int error_buffer_size)
{
  ConvertOptions options;
  return runWithStore(input_rvm_path,
                      input_att_path,
                      options,
                      output_glb_path,
                      output_json_path,
                      nullptr,
                      error_buffer,
                      error_buffer_size);
}

RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_glb_file_ex(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_glb_path,
    const char* output_json_path,
    float tolerance,
    int rotate_z_to_y,
    int center_model,
    int include_attributes,
    int merge_geometries,
    char* error_buffer,
    int error_buffer_size)
{
  ConvertOptions options;
  options.tolerance = tolerance;
  options.rotateZToY = rotate_z_to_y != 0;
  options.centerModel = center_model != 0;
  options.includeAttributes = include_attributes != 0;
  options.mergeGeometries = merge_geometries != 0;

  return runWithStore(input_rvm_path,
                      input_att_path,
                      options,
                      output_glb_path,
                      output_json_path,
                      nullptr,
                      error_buffer,
                      error_buffer_size);
}

RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_rev_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_rev_path,
    char* error_buffer,
    int error_buffer_size)
{
  ConvertOptions options;
  return runWithStore(input_rvm_path,
                      input_att_path,
                      options,
                      nullptr,
                      nullptr,
                      output_rev_path,
                      error_buffer,
                      error_buffer_size);
}

RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_json_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_json_path,
    char* error_buffer,
    int error_buffer_size)
{
  ConvertOptions options;
  return runWithStore(input_rvm_path,
                      input_att_path,
                      options,
                      nullptr,
                      output_json_path,
                      nullptr,
                      error_buffer,
                      error_buffer_size);
}

} // extern "C"
