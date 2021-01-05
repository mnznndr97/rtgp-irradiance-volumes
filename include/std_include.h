#pragma once

#include <Platform.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>


#include<utils/shader_v1.h>
#include<utils/model_v2.h>
#include<utils/camera.h>

#include <ft2build.h>
#include FT_FREETYPE_H 

#ifndef _DEBUG_DRAW
#define _DEBUG_DRAW
#endif // !_DEBUG_DRAW


#define NO_COPY_AND_ASSIGN(T) \
  T(const T&) = delete;   \
  void operator=(const T&) = delete


#define DEFAULT_COPY_AND_ASSIGN(T) \
  T(const T&) = default;   \
  void operator=(const T&) = default


template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec3)
{
    os << "x: " << vec3.x << ", y: " << vec3.y << "z: " << vec3.z;
    return os;
}


namespace std
{
    template<> struct hash<glm::vec3>
    {
        std::size_t operator()(glm::vec3 const& s) const noexcept
        {
            std::size_t seed = 0;
            hash_combine(seed, s.x);
            hash_combine(seed, s.y);
            hash_combine(seed, s.z);
            return seed;
        }
    };
}

inline void DebugBreak(){
   #ifdef ISWINPLATFORM
        __debugbreak();
   #else 
   #endif     
}
