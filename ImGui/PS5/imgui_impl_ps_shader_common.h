/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 10.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc.
* 
*/
#ifndef IMGUI_IMPL_PS_SHADER_COMMON_H
#define IMGUI_IMPL_PS_SHADER_COMMON_H

namespace ImGui_PS {
namespace EmbeddedShader {

#if defined(__cplusplus)
    struct float2 {
        union {
            struct {
                float x, y;
            };
            struct {
                float r, g;
            };
            struct {
                float u, v;
            };
            float s[2];
        };

        float2() : x(0), y(0) {}
        float2(float xx, float yy) : x(xx), y(yy) {}
    };

    struct float3 {
        union {
            struct {
                float x, y, z;
            };
            struct {
                float r, g, b;
            };
            float s[3];
        };

        float3() : x(0), y(0), z(0) {}
        float3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    };

    struct float4 {
        union {
            struct {
                float x, y, z, w;
            };
            struct {
                float r, g, b, a;
            };
            float s[4];
        };

        float4() : x(0), y(0), z(0), w(0) {}
        float4(float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) {}
    };

    struct float4x4 {
        union {
            struct {
                float m00, m10, m20, m30;
                float m01, m11, m21, m31;
                float m02, m12, m22, m32;
                float m03, m13, m23, m33;
            };
            struct {
                float4 c[4];
            };
            float s[16];
        };

        float4x4(const float v[16]) :
            c{ {v[0], v[1], v[2], v[3]}, {v[4], v[5], v[6], v[7]}, {v[8], v[9], v[10], v[11]}, {v[12], v[13], v[14], v[15]} } {}
        float4x4(const float4 &c0, const float4 &c1, const float4 &c2, const float4 &c3) :
            c{ c0, c1, c2, c3 } {}
    };

#   if defined(__PROSPERO__)
    template <typename T>
    using Texture2D = sce::Agc::Core::Texture;
    using SamplerState = sce::Agc::Core::Sampler;
#   endif

#   define PSSL_ONLY(var)
#   define PSSL_SEMANTIC(var)
#elif defined(__PSSL__)
#   define PSSL_ONLY(var) var
#   define PSSL_SEMANTIC(var) : var

#   if defined(__PROSPERO__)
#       define CXX_SYMBOL(basename) [CxxSymbol(basename)]
#   endif
#endif

    typedef float4x4 Matrix4x4;
    typedef float4 RGBA;

    // NOTE: This must match ImDrawVert.
    struct Attribute {
        float2 posInModel PSSL_SEMANTIC(POSITION_IN_MODEL);
        float2 texCoord PSSL_SEMANTIC(TEXCOORD);
#if defined(__cplusplus)
        uint32_t color;
#else
        RGBA color PSSL_SEMANTIC(COLOR);
#endif
        float depth_padding;
    };

    struct Varying {
        float4 posSys PSSL_SEMANTIC(S_POSITION);
        float2 texCoord PSSL_SEMANTIC(TEXCOORD);
        RGBA color PSSL_SEMANTIC(COLOR);
    };

    struct CBPerFrame {
        Matrix4x4 matProj;
    };

    struct CBPerObject {
        Texture2D<RGBA> texture;
        SamplerState sampler;
    };

    struct SRT {
        CBPerFrame* cbPerFrame;
        CBPerObject* cbPerObject;
    };

}
}

#endif
