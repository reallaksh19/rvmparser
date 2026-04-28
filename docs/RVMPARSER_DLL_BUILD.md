# RVM Parser DLL Build and Desktop Integration

This branch adds a native DLL/shared-library target around the existing `rvmparser` source code.

## What was added

- `src/RvmParserDll.h` — stable C ABI for desktop callers.
- `src/RvmParserDll.cpp` — DLL implementation using the real parser/export pipeline.
- `CMakeLists.txt` — builds both:
  - `rvmparser` command-line executable
  - `rvm_parser.dll` / `librvm_parser.so` / `librvm_parser.dylib`

## Exported DLL functions

```cpp
const char* rvmparser_dll_version();

int rvmparser_convert_rvm_to_glb_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_glb_path,
    const char* output_json_path,
    char* error_buffer,
    int error_buffer_size);

int rvmparser_convert_rvm_to_glb_file_ex(
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

int rvmparser_convert_rvm_to_rev_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_rev_path,
    char* error_buffer,
    int error_buffer_size);

int rvmparser_convert_rvm_to_json_file(
    const char* input_rvm_path,
    const char* input_att_path,
    const char* output_json_path,
    char* error_buffer,
    int error_buffer_size);
```

## Build on Windows

Install Visual Studio with C++ workload and CMake, then run from repo root:

```bat
cmake -S . -B build -A x64
cmake --build build --config Release --target rvm_parser
```

Expected output:

```text
build\Release\rvm_parser.dll
build\Release\rvm_parser.lib
```

## Build original EXE too

```bat
cmake --build build --config Release --target rvmparser
```

Expected output:

```text
build\Release\rvmparser.exe
```

## VB.NET P/Invoke example

```vbnet
Imports System.Runtime.InteropServices
Imports System.Text

Public Class NativeRvmParser
    <DllImport("rvm_parser.dll", CallingConvention:=CallingConvention.Cdecl, CharSet:=CharSet.Ansi)>
    Public Shared Function rvmparser_convert_rvm_to_glb_file(
        inputRvmPath As String,
        inputAttPath As String,
        outputGlbPath As String,
        outputJsonPath As String,
        errorBuffer As StringBuilder,
        errorBufferSize As Integer
    ) As Integer
    End Function
End Class
```

Call:

```vbnet
Dim errorBuffer As New StringBuilder(8192)
Dim code = NativeRvmParser.rvmparser_convert_rvm_to_glb_file(
    inputPath,
    Nothing,
    outputGlbPath,
    outputJsonPath,
    errorBuffer,
    errorBuffer.Capacity
)

If code <> 0 Then
    Throw New Exception(errorBuffer.ToString())
End If
```

## Notes

- The DLL currently uses file-path input and output. This is simpler and safer for VB.NET/C# integration than cross-boundary memory ownership.
- `input_att_path` is optional. Pass `Nothing` / `null` / empty string when no attribute file is available.
- The DLL uses the same parser/export logic as the command-line program: `parseRVM`, optional `parseAtt`, `connect`, `align`, `AddGroupBBox`, `Tessellator`, `exportGLTF`, `exportJson`, and `exportRev`.
- For a WebView2 desktop shell, place `rvm_parser.dll` beside the desktop executable or configure the DLL search path to a `Native` folder.
