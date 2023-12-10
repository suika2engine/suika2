#ifndef GameShaderTypes_h
#define GameShaderTypes_h

#include <simd/simd.h>

enum GameVertexInputIndex {
    GameVertexInputIndexVertices = 0,
};

enum {
    GameTextureIndexBaseColor = 0,
    GameTextureIndexRuleLevel = 1,
};

struct GameVertex {
    vector_float2 xy;
    vector_float2 uv;
    float alpha;
    float padding;
};

#endif /* GameShaderTypes_h */
