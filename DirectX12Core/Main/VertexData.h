#pragma once
#include <vector>
#include "Mesh.h"

// 샘플 정점 데이터를 반환합니다.
inline std::vector<Vertex> GetCubeVertices() {
    return {
        // Front face
        {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},  // 0: Bottom-left-front
        {{1.0f, -1.0f, -1.0f},  {0.0f, 1.0f, 0.0f, 1.0f}},  // 1: Bottom-right-front
        {{1.0f, 1.0f, -1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}},  // 2: Top-right-front
        {{-1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, 0.0f, 1.0f}},  // 3: Top-left-front

        // Back face
        {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f, 1.0f}},  // 4: Bottom-left-back
        {{1.0f, -1.0f, 1.0f},   {0.0f, 1.0f, 1.0f, 1.0f}},  // 5: Bottom-right-back
        {{1.0f, 1.0f, 1.0f},    {1.0f, 0.5f, 0.0f, 1.0f}},  // 6: Top-right-back
        {{-1.0f, 1.0f, 1.0f},   {0.5f, 0.0f, 0.5f, 1.0f}},  // 7: Top-left-back
    };
}

// 샘플 인덱스 데이터를 반환합니다.
inline std::vector<uint16_t> GetCubeIndices() {
    return {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 6, 5, 4, 7, 6,
        // Left face
        4, 0, 3, 4, 3, 7,
        // Right face
        1, 5, 6, 1, 6, 2,
        // Top face
        3, 2, 6, 3, 6, 7,
        // Bottom face
        4, 5, 1, 4, 1, 0,
    };
}