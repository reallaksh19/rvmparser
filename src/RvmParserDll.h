#pragma once

// Public C ABI for using rvmparser from desktop applications such as
// VB.NET/C#/WebView2 shells. Keep this header ABI-safe: no std::string,
// no C++ classes, and no ownership transfer across the DLL boundary.

#ifdef _WIN32
  #ifdef RVMPARSER_DLL_BUILD
    #define RVMPARSER_DLL_API __declspec(dllexport)
  #else
    #define RVMPARSER_DLL_API __declspec(dllimport)
  #endif
  #define RVMPARSER_DLL_CALL __cdecl
#else
  #define RVMPARSER_DLL_API __attribute__((visibility("default")))
  #define RVMPARSER_DLL_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Return a static version string. Do not free the returned pointer.
RVMPARSER_DLL_API const char* RVMPARSER_DLL_CALL rvmparser_dll_version();

// Convert one RVM file, optionally with one ATT/TXT attribute sidecar, to GLB.
// output_json_path is optional: pass nullptr or an empty string to skip JSON export.
// Returns 0 on success. Non-zero values indicate failure.
RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_glb_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_glb_path,
    const char* output_json_path,
    char* error_buffer,
    int error_buffer_size);

// Advanced GLB conversion. Boolean arguments are int for ABI simplicity.
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
    int error_buffer_size);

// Convert one RVM file, optionally with one ATT/TXT attribute sidecar, to REV text.
RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_rev_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_rev_path,
    char* error_buffer,
    int error_buffer_size);

// Convert one RVM file, optionally with one ATT/TXT attribute sidecar, to JSON only.
RVMPARSER_DLL_API int RVMPARSER_DLL_CALL rvmparser_convert_rvm_to_json_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_json_path,
    char* error_buffer,
    int error_buffer_size);

#ifdef __cplusplus
}
#endif
