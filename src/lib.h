#pragma once

#ifdef VK_ZERO_CPU

#define VK_NO_PROTOTYPES
#include "SDL.h"
#include "SDL_vulkan.h"

#include "VkBootstrap.h"

#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "volk.h"

#endif
