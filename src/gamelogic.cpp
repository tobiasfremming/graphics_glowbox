#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;
SceneNode* light1Node;
SceneNode* light2Node;
SceneNode* movingLightNode;
SceneNode* node2d;
SceneNode* rootNode2D;

unsigned int textVAO;        // VAO for text rendering
unsigned int charmapTextureID; // Texture ID for the font atlas
unsigned int brickTextureID;
unsigned int brickNormalTextureID;
int roughnessTextureID;
Mesh textMesh;               // Store the text mesh globally

glm::mat4 VP;
glm::mat4 P;
glm::mat4 OrthoProjection;


double ballRadius = 3.0f;
glm::vec3 cameraPos;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shader2D;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

CommandLineOptions options;

bool hasStarted        = false;
bool hasLost           = false;
bool jumpedToNextFrame = false;
bool isPaused          = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}



unsigned int createTexture(const PNGImage& image){
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID); // bind texture

    glTexImage2D(
                GL_TEXTURE_2D,   // target
                 0,              // mipmap level
                 GL_RGBA,        // internalFormat (store as RGBA)
                 image.width,
                 image.height,
                 0,              // border (must be 0)
                 GL_RGBA,        // inputFormat (matches internalFormat)
                 GL_UNSIGNED_BYTE,
                 image.pixels.data());

    

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glGenerateMipmap(GL_TEXTURE_2D); // mipmap

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture 

    // Check for errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }

    return textureID;

}









//// A few lines to help you if you've never used c++ structs
struct LightSource {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    // int id; // vertex array object id from its node
};
LightSource lightSources[3];


void setUniforms(){
    GLuint shaderProgram = shader->get();
    glUniform3fv(glGetUniformLocation(5, "viewPos"), 1, glm::value_ptr(cameraPos));
    glUniform3f(glGetUniformLocation(shaderProgram, "ball_position"), ballNode->position.x, ballNode->position.y, ballNode->position.z);
    glUniform1f(glGetUniformLocation(shaderProgram, "ball_radius"), ballRadius);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "VP"), 1, GL_FALSE, glm::value_ptr(VP));

    for (int i = 0; i < 3; i++) {
        std::string posUniform   = "lights[" + std::to_string(i) + "].position";
        std::string colorUniform = "lights[" + std::to_string(i) + "].color";

        glUniform3f(glGetUniformLocation(shaderProgram, posUniform.c_str()), lightSources[i].position.x, lightSources[i].position.y, lightSources[i].position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, colorUniform.c_str()), lightSources[i].color.x, lightSources[i].color.y, lightSources[i].color.z);
    }

}

void setUniforms2D(){
    GLuint shaderProgram = shader2D->get();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "OrthoProjection"), 1, GL_FALSE, glm::value_ptr(OrthoProjection));
    
    
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    PNGImage brickTexture = loadPNGFile("../res/textures/Brick03_col.png");
    brickTextureID = createTexture(brickTexture);

    PNGImage brickNormalTexture = loadPNGFile("../res/textures/Brick03_nrm.png");
    brickNormalTextureID = createTexture(brickNormalTexture);

    PNGImage roughnessTexture = loadPNGFile("../res/textures/Brick03_rgh.png");
    roughnessTextureID = createTexture(roughnessTexture);




    // Loading the texture
    PNGImage charmap = loadPNGFile("../res/textures/charmap.png");
    charmapTextureID = createTexture(charmap);

    float characterAspectRatio = 39.0f / 29.0f;
    float textWidth = 12.0f;  // Adjust this based on the desired size
    textMesh = generateTextGeometryBuffer("HELLO WORLD!", characterAspectRatio, textWidth*29);
    unsigned int textMeshVAO = generateBuffer(textMesh);

    node2d = new SceneNode();
    node2d->nodeType = NODE2D;
    node2d->textureID = charmapTextureID;
    node2d->vertexArrayObjectID = textMeshVAO;
    node2d->VAOIndexCount = textMesh.indices.size();

    // node2D->position = glm::vec3(-15.0f, -50.0f, -90.0f);
    
    
    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    shader->activate();

    // Load the 2D UI shader
    shader2D = new Gloom::Shader();
    shader2D->makeBasicShader("../res/shaders/shader2d.vert", "../res/shaders/shader2d.frag");
    
    OrthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight), -1.0f, 1.0f);

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);

    // Construct scene
    rootNode = createSceneNode();
    boxNode  = createSceneNode();
    padNode  = createSceneNode();
    ballNode = createSceneNode();

    light1Node = createSceneNode();
    light2Node = createSceneNode();
    movingLightNode = createSceneNode();

    rootNode2D = createSceneNode();

    light1Node->nodeType = POINT_LIGHT;
    light2Node->nodeType = POINT_LIGHT;
    movingLightNode->nodeType = POINT_LIGHT;

    // Set initial positions of the lights
    light1Node->position = glm::vec3(0.0f, 0.0f, 0.0f); // Static
    light2Node->position = glm::vec3(-15.0f, -50.0f, -90.0f);   // Static
    movingLightNode->position = glm::vec3(0.0f, -20.0f, -10.0f); // Moving

    rootNode2D->children.push_back(node2d);

    rootNode->children.push_back(light1Node);
    rootNode->children.push_back(light2Node);
    rootNode->children.push_back(movingLightNode);

    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(padNode);
    rootNode->children.push_back(ballNode);

    boxNode->vertexArrayObjectID  = boxVAO;
    boxNode->VAOIndexCount        = box.indices.size();
    boxNode->nodeType             = NORMAL_MAPPED;
    boxNode->textureID = brickTextureID;
    boxNode->normalMapID = brickNormalTextureID;
    boxNode->roughnessTextureID = roughnessTextureID;

    padNode->vertexArrayObjectID  = padVAO;
    padNode->VAOIndexCount        = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount       = sphere.indices.size();

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;
    
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    shader->activate();
    GLuint shaderProgram = shader->get();

    double timeDelta = getTimeDeltaSeconds();

    // Move the dynamic light in a circular motion
    float lightMoveRadius = 40.0f;
    float lightSpeed = 0.5f;
    movingLightNode->position.x = sin(glfwGetTime() * lightSpeed) * lightMoveRadius;
    movingLightNode->position.z = cos(glfwGetTime() * lightSpeed) * lightMoveRadius;
    
    
    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
        

    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 20.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ
                ) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    glm::vec3 cameraPosition = glm::vec3(0, 2, -20);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    glm::mat4 cameraTransform =
                    glm::rotate(0.3f + 0.2f * float(-padPositionZ*padPositionZ), glm::vec3(1, 0, 0)) *
                    glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
                    glm::translate(-cameraPosition);

    VP = projection * cameraTransform;
    P = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

   
    // glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "VP"), 1, GL_FALSE, glm::value_ptr(VP));


    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
    };

    
    // glUniform3f(glGetUniformLocation(shaderProgram, "ball_position"), ballNode->position.x, ballNode->position.y, ballNode->position.z);
    // glUniform1f(glGetUniformLocation(shaderProgram, "ball_radius"), ballRadius);

    light1Node->position = ballPosition;
    light1Node->position.z += 20;

    
    
    // Send light positions to the shader
    // glm::vec3 lightPositions[3] = {
    //     light1Node->position,
    //     light2Node->position,
    //     movingLightNode->position
    // };
    glm::vec3 lightPositions[3] = {
        glm::vec3(light1Node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1)),
        glm::vec3(light2Node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1)),
        glm::vec3(movingLightNode->currentTransformationMatrix * glm::vec4(0, 0, 0, 1))
    };
    glm::vec3 lightColors[3] = {
        glm::vec3(0.29, 0.72, 0.59),  // R light
        glm::vec3(0.86, 0.28, 0.64),  // G light
        glm::vec3(1.0, 0.85, 0.0)   // B moving light
    };

    for (int i = 0; i < 3; i++){
        lightSources[i].position = lightPositions[i];
        lightSources[i].color = lightColors[i];
    }
    


    // Pass light data to the shader
    // GLuint shaderProgram = shader->getProgramID();
    // glUseProgram(shaderProgram);
    
    if (!shader->isValid()) {
    std::cerr << "Shader program failed validation!" << std::endl;
    }
    

    

    // Update camera position for specular reflections
    cameraPos = glm::vec3(0, 2, -20);
    // glUniform3fv(glGetUniformLocation(5, "viewPos"), 1, glm::value_ptr(cameraPos));

    updateNodeTransformations(rootNode, glm::mat4(1.0f));
    updateNodeTransformations(rootNode2D, glm::mat4(1.0f));

}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;

    switch(node->nodeType) {
        case GEOMETRY: 
            //glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
            break;
        case POINT_LIGHT: 
            break;
        case SPOT_LIGHT: 
            break;
        case NORMAL_MAPPED:
            break;
        
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    GLuint shaderProgram = shader->get();
    glm::mat4 modelMatrix = node->currentTransformationMatrix;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    shaderProgram = shader2D->get();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));


    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = shader->getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 0);
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
        case NODE2D:
            // glUseProgram(shaderProgram);
            // glBindTexture(GL_TEXTURE_2D, node->textureID);
            glBindTextureUnit(0, node2d->textureID);
            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            // glBindVertexArray(0);
            
            break;
        case NORMAL_MAPPED: {

        
            GLint hasTextureLocation = shader->getUniformFromName("hasTexture");
            glUniform1i(hasTextureLocation, 1);

            glBindTextureUnit(0, node->textureID);
            glBindTextureUnit(1, node->normalMapID);
            glBindTextureUnit(2, node->roughnessTextureID);

            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

            break;

        }
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    shader->activate();
    setUniforms();
    renderNode(rootNode);

    shader2D->activate();
    setUniforms2D();
    renderNode(rootNode2D);
    
}
