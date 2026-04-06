# BlenderSI Workflow Brief

This is a local handoff note for this workspace. It is not upstream Blender documentation.

## Current Layout

- Source repo: `C:\dev\BlenderSI\blender`
- Build dir: `C:\dev\BlenderSI\build_windows_vs2026`
- OptiX SDK headers: `C:\dev\BlenderSI\optix-sdk-8.1.0`

## Current Git State

- Local branch: `anisotropic-refraction`
- `origin`: `https://github.com/DGruwier/blender-anisotropic-refraction.git`
- `upstream`: `https://github.com/blender/blender.git`
- `upstream` push URL is intentionally disabled

## Toolchain That Works Here

- Visual Studio Community 2026
- MSVC `19.50`
- CUDA Toolkit `12.8`
- OptiX SDK headers `8.1.0` from `NVIDIA/optix-sdk`

Important:

- Plain Blender C++ builds work with VS 2026.
- CUDA/OptiX kernel builds do not officially support VS 2026 with CUDA 12.8.
- This workspace works around that by using `CUDA_NVCC_FLAGS=--allow-unsupported-compiler`.
- There is also a local build-system patch in `intern/cycles/kernel/device/optix/CMakeLists.txt` so OptiX kernel builds honor `CUDA_NVCC_FLAGS`.

If that workaround ever becomes unstable, the clean fallback is VS 2022 Build Tools for GPU builds.

## One-Time Setup Already Done On This Machine

CUDA Toolkit install used:

```powershell
winget install --id Nvidia.CUDA --version 12.8 --accept-package-agreements --accept-source-agreements --override "-s -n nvcc_12.8 cudart_12.8 nvrtc_12.8 nvrtc_dev_12.8 nvjitlink_12.8 nvfatbin_12.8"
```

OptiX SDK headers used:

```powershell
git clone --depth 1 --branch v8.1.0 https://github.com/NVIDIA/optix-sdk.git C:\dev\BlenderSI\optix-sdk-8.1.0
```

## Build Configuration That Works

This build is configured for the GPUs that matter here:

- `sm_75` for RTX 2070
- `sm_120` for RTX 5070 Ti
- `compute_75` PTX fallback

If reconfiguring an existing build dir, use:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' `
  -S 'C:\dev\BlenderSI\blender' `
  -B 'C:\dev\BlenderSI\build_windows_vs2026' `
  -DCUDA_NVCC_FLAGS='--allow-unsupported-compiler' `
  -DCYCLES_CUDA_BINARIES_ARCH='sm_75;sm_120;compute_75' `
  -DOPTIX_ROOT_DIR='C:/dev/BlenderSI/optix-sdk-8.1.0' `
  -DOPTIX_INCLUDE_DIR='C:/dev/BlenderSI/optix-sdk-8.1.0/include' `
  -DCYCLES_RUNTIME_OPTIX_ROOT_DIR='C:/dev/BlenderSI/optix-sdk-8.1.0' `
  -DCUDA_TOOLKIT_ROOT_DIR='C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8' `
  -DWITH_CYCLES_CUDA_BINARIES=ON `
  -DWITH_CYCLES_DEVICE_OPTIX=ON
```

If the build dir is missing and you need to bootstrap it first:

```powershell
$env:PATH='C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;' + $env:PATH
cmd /d /s /c "make.bat developer 2026 builddir build_windows_vs2026 nobuild"
```

Then run the reconfigure command above.

## Safe Build Loop

Do not delete the build dir unless it is actually broken.

Safest command for day-to-day work:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' `
  --build 'C:\dev\BlenderSI\build_windows_vs2026' `
  --config Release `
  --target install `
  -- /m:4 /nodeReuse:false
```

Why `install` and not only `blender`:

- it refreshes the runtime payload
- it installs Cycles CUDA/OptiX kernels into `5.2/scripts/addons_core/cycles/lib`
- it avoids stale runtime layout issues

## Launch Blender

```powershell
& 'C:\dev\BlenderSI\build_windows_vs2026\bin\Release\blender.exe'
```

## Quick GPU Sanity Check

Headless device probe:

```powershell
$script = @'
import _cycles
print('DEVICE_TYPES', _cycles.get_device_types())
for t in ['CPU', 'CUDA', 'OPTIX']:
    print(t, _cycles.available_devices(t))
'@
$tmp = Join-Path $env:TEMP 'blender_cycles_probe.py'
Set-Content -LiteralPath $tmp -Value $script -Encoding ASCII
& 'C:\dev\BlenderSI\build_windows_vs2026\bin\Release\blender.exe' --background --factory-startup --python $tmp
Remove-Item -LiteralPath $tmp -Force
```

Expected result in this workspace:

- `CUDA` shows `NVIDIA GeForce RTX 2070`
- `OPTIX` shows `NVIDIA GeForce RTX 2070`

UI check:

1. Open Blender.
2. Go to `Edit > Preferences > System`.
3. Under `Cycles Render Devices`, verify `CUDA` and `OptiX` are available.

## Current Feature Work

Current renderer experiment branch is `anisotropic-refraction`.

Custom placeholder node work is in these areas:

- `source/blender/nodes/shader/nodes/node_shader_bsdf_anisotropic_refraction.cc`
- `source/blender/gpu/shaders/material/gpu_shader_material_anisotropic_refraction.glsl`
- `intern/cycles/blender/shader.cpp`

Current placeholder behavior in Cycles:

- the custom node mixes refraction with green emission using the `Anisotropy` input

## If GPU Support Breaks Again

Check these first:

1. `nvcc --version`
2. `C:\dev\BlenderSI\optix-sdk-8.1.0\include\optix.h`
3. `CMakeCache.txt` contains:
   - `WITH_CYCLES_CUDA_BINARIES:BOOL=ON`
   - `WITH_CYCLES_DEVICE_OPTIX:BOOL=ON`
   - `CUDA_NVCC_FLAGS:STRING=--allow-unsupported-compiler`
   - `OPTIX_INCLUDE_DIR` pointing to `optix-sdk-8.1.0\include`
4. `5.2/scripts/addons_core/cycles/lib` contains:
   - `kernel_sm_75.cubin.zst`
   - `kernel_sm_120.cubin.zst`
   - `kernel_optix*.ptx.zst`

If CUDA works but OptiX disappears, check the local patch in `intern/cycles/kernel/device/optix/CMakeLists.txt` first.
