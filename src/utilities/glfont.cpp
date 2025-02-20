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

    const int numCols = 16; // Assuming 16 characters per row
    const int numRows = 8;  // Assuming 8 rows

    float charWidthUV = 1.0f / numCols; 
    float charHeightUV = 1.0f / numRows;

    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        int asciiIndex = static_cast<int>(text[i]);

        int charCol = asciiIndex % numCols;
        int charRow = asciiIndex / numCols;

        // float u0 = charCol * charWidthUV;         // Left
        // float v0 = charRow * charHeightUV;        // Bottom
        // float u1 = u0 + charWidthUV;              // Right
        // float v1 = v0 + charHeightUV;             // Top

        float u0 = charCol * charWidthUV;         // Left
        float v0 = (numRows - charRow - 1) * charHeightUV;  // Correcting flipped image
        float u1 = u0 + charWidthUV;              // Right
        float v1 = v0 + charHeightUV;
        


        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};                    // Bottom-left
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};   // Bottom-right
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0}; // Top-right
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};      // Top-left

        // Assign texture coordinates (UV mapping)
        mesh.textureCoordinates.at(4 * i + 0) = {u0, v1}; // Bottom-left
        mesh.textureCoordinates.at(4 * i + 1) = {u1, v1}; // Bottom-right
        mesh.textureCoordinates.at(4 * i + 2) = {u1, v0}; // Top-right
        mesh.textureCoordinates.at(4 * i + 3) = {u0, v0}; // Top-left

        // Define indices for two triangles forming the quad
        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;



        // mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        // mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        // mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        // mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        // mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        // mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};

    }

    return mesh;
}