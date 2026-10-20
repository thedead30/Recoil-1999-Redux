// upscale.h - Setup's "Generate HD textures on this PC" (Redux v1, user 2026-10-20): a Redux texture pack made from the
// player's own game files with Real-ESRGAN (payload\esrgan), or a smooth enlargement when Real-ESRGAN cannot run.
// Not Recoil code (PLATFORM).
#pragma once

#include <functional>
#include <string>

namespace setup {

// Builds <dest>\redux_textures from <dest>\zbd\m1..m13 (texture6.zbd colour, rtexture4.zbd alpha). esrgan_dir holds
// realesrgan-ncnn-vulkan.exe and models\. Returns the number of textures written; log/progress as Setup's.
int GenerateTextures(const std::wstring& dest, const std::wstring& esrgan_dir, const std::function<void(const std::string&)>& log,
                     const std::function<void(int, int)>& progress);

}  // namespace setup
