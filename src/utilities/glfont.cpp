#include <iostream>
#include "glfont.h"

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth) {
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);
    mesh.textureCoordinates.resize(vertexCount);


    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;
        char character = text[i];

        // uv coords
        float u_start = (character % 16) / 16.0f;  // Column
        float v_start = (character / 16) / 8.0f;   // Row
        float u_end = u_start + (1.0f / 16.0f);
        float v_end = v_start + (1.0f / 8.0f);

        // mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        // mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        // mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        // mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        // mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        // mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};                  // Bottom-left
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0}; // Bottom-right
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0}; // Top-right
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};   // Top-left

        mesh.textureCoordinates.at(4 * i + 0) = {u_start, v_end}; // Bottom-left
        mesh.textureCoordinates.at(4 * i + 1) = {u_end, v_end};   // Bottom-right
        mesh.textureCoordinates.at(4 * i + 2) = {u_end, v_start}; // Top-right
        mesh.textureCoordinates.at(4 * i + 3) = {u_start, v_start}; // Top-left


        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;
    }

    return mesh;
}