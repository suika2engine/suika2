// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*-

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

//
// ShaderTypes: The Metal version of the shaders for Suika2
//

#ifndef SUIKA_SHADERTYPES_H
#define SUIKA_SHADERTYPES_H

#include <simd/simd.h>

enum ShaderVertexInputIndex {
    ShaderVertexInputIndexVertices = 0,
};

enum {
    ShaderTextureIndexColor = 0,
    ShaderTextureIndexRule = 1,
};

struct ShaderVertex {
    vector_float2 xy;
    vector_float2 uv;
    float alpha;
    float padding;  // This is absolutely needed for 64-bit alignments
};

#endif
