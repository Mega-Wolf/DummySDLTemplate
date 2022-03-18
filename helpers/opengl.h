#pragma once

// TODO(Tobi): The OpenGL stuff will probably be changed several times, so I don't care about it all too much now

#include "../code/macros.h"
#include "debug_functions.h"
#include "../code/bitmap.h"
#include "../code/drawing.h"
#include "../extern/glad/glad.h"
#include "../extern/HandmadeMath.h"

#include "profiler.h"

#include <time.h>

#define RENDER_LAYER_COUNT 10
#define RENDER_LAYER_MAX_SIZE 4096 // TODO(Tobi): How many objects can be on one layer; obviously change that please

void GLCHK() {
    int errorCode = glGetError();
    //Assert(!errorCode, "Error: %d - %x", errorCode, errorCode);
}

struct debug_shader_info {
    char* VertexPath;
    char* FragmentPath;
    uint32 ProgramID;
    uint32 VertexID;
    uint32 FragmentID;
};

int DebugShaderInfosCount;
debug_shader_info DebugShaderInfos[100];

struct color4f {
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

// TODO(Tobi): I am not sure about that
struct sprite {
    uint32 TextureID;
    uint32 VAO;
};

struct sprite_data {
    sprite Sprite;
    float Left;
    float Right;
    float Top;
    float Bottom;
    loaded_bitmap LoadedBitmap;
};

struct render_object {
    hmm_mat4 ModelMatrix; // TODO(Tobi): This is not correct, but I later have to get more clever about this anyway (mat4 + model matrix)

    sprite Sprite; // This could be made a pointer so that it keeps consistent
    uint32 ShaderID;
    color4f Color;
};

struct ogl_data {
    // This part is global
    uint32 BrokenShader;
    uint32 BasicShader;
    uint32 BasicAlphaCutoffShader;
    uint32 SimpleShader;
    uint32 OutlineShader;
    uint32 DiscShader;
    uint32 CircleShader;

    uint32 BackgroundShader1;
    uint32 BackgroundShader2;
    uint32 GaussianBlurShader;
    uint32 BumpToNormalShader;

    uint32 ManaBarShader;

    uint32 DummyVAO;
    uint32 BlockIDMatrices;
    uint32 UniformTime;

    sprite_data BrokenSprite;
    sprite_data WhiteSprite;

    sprite_data TopLeftUnitSprite;

    // ogl_context* ActiveContext;

    // // TODO(Tobi): These will mostly go (unless debugging etc.)
    // LIST(render_object, 4096 * 256) IMRenderObjects;
    // LIST(render_object, 4096 * 256) IMRenderObjectsScreenspace;

    // NOTE(Tobi): Actually, I don't really have to care about the sorting tbh
    int LayerSizes[RENDER_LAYER_COUNT];
    render_object LayerData[RENDER_LAYER_COUNT][RENDER_LAYER_MAX_SIZE];
    draw_rect LayerDrawRects[RENDER_LAYER_COUNT];

    uint32 FrameBuffers[2]; // NOTE(Tobi): I just use two ful screen FBOs for now
    uint32 FrameBufferTextures[2]; // TODO(Tobi): Apparently I should rather create them again and again

    float RunningTime;
};

ogl_data OGLData;

bool _LoadAndCompileShader(uint32 id, char* path) {
    debug_read_file_result result = DebugPlatformReadEntireTextFile(path);
    if (result.Size == 0) { return false; }

    char* data = (char*) result.Content;
    glShaderSource(id, 1, &data, NULL);
    glCompileShader(id);
    // check for shader compile errors
    {
        int success;
        char infoLog[512];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            Assert(false, "Error: Shader compilation failed\n%s\n", infoLog);
            return false;
        }
    }
    return true;
}

// TODO(Tobi): I might want to link to additional shaders
// Also, I might share vertex or fragment shaders so I want to do that step separately
// Therefore, this thing will go at one point again
int LoadShader(char* vertexShaderPath, char* fragmentShaderPath) {
    uint32 programID = glCreateProgram();
    uint32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    uint32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // TODO(Tobi): React on errors
    _LoadAndCompileShader(vertexShader, vertexShaderPath);
    _LoadAndCompileShader(fragmentShader, fragmentShaderPath);

    DebugShaderInfos[DebugShaderInfosCount].ProgramID    = programID;
    DebugShaderInfos[DebugShaderInfosCount].VertexID     = vertexShader;
    DebugShaderInfos[DebugShaderInfosCount].FragmentID   = fragmentShader;
    DebugShaderInfos[DebugShaderInfosCount].VertexPath   = vertexShaderPath;
    DebugShaderInfos[DebugShaderInfosCount].FragmentPath = fragmentShaderPath;
    ++DebugShaderInfosCount;

    GLCHK();

    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    GLCHK();

    // check for linking errors

    uint32 returnID = programID;
    {
        int success;
        char infoLog[512];
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(programID, 512, NULL, infoLog);
            Assert(false, "Error: Shader LINKING failed\n%s\nVertex: %s\nFragment: %s\n", infoLog, vertexShaderPath, fragmentShaderPath);

            returnID = OGLData.BrokenShader; 
        } else {
            // NOTE(Tobi): I just assume that this is the case everywhere
            glUseProgram(programID);

            // TODO(Tobi): What does this even mean (same for original place)
            //glUniform1i(glGetUniformLocation(programID, "texture1"), 0);
            glUniformBlockBinding(programID, glGetUniformBlockIndex(programID, "Matrices"), 0);
            int timeIndex = glGetUniformBlockIndex(programID, "Time");
            if (timeIndex != -1) {
                glUniformBlockBinding(programID, timeIndex, 1);
            }
        }
    }
    GLCHK();

    // TODO(Tobi): Does this change anything
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLCHK();
    return returnID;
}

void RecompileAllShaders() {
    inc0 (shader_i,   DebugShaderInfosCount) {
        debug_shader_info dsi = DebugShaderInfos[shader_i];
        _LoadAndCompileShader(dsi.FragmentID, dsi.FragmentPath);
        _LoadAndCompileShader(dsi.VertexID, dsi.VertexPath);

        // NOTE(Tobi): Those are already attached; I don't have to do that again
        //glAttachShader(programID, vertexShader);
        //glAttachShader(programID, fragmentShader);
        glLinkProgram(dsi.ProgramID);
        // check for linking errors

        uint32 returnID = dsi.ProgramID;
        {
            int success;
            char infoLog[512];
            glGetProgramiv(dsi.ProgramID, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(dsi.ProgramID, 512, NULL, infoLog);
                Assert(false, "Error: Shader LINKING failed\n%s\nVertex: %s\nFragment: %s\n", infoLog, dsi.VertexPath, dsi.FragmentPath);

                returnID = OGLData.BrokenShader; 
            } else {
                // NOTE(Tobi): I just assume that this is the case everywhere
                glUseProgram(dsi.ProgramID);
                glUniform1i(glGetUniformLocation(dsi.ProgramID, "texture1"), 0);
                glUniformBlockBinding(dsi.ProgramID, glGetUniformBlockIndex(dsi.ProgramID, "Matrices"), 0);
                int timeIndex = glGetUniformBlockIndex(dsi.ProgramID, "Time");
                if (timeIndex != -1) {
                    glUniformBlockBinding(dsi.ProgramID, timeIndex, 1);
                }
            }
        }

        glDeleteShader(dsi.VertexID);
        glDeleteShader(dsi.FragmentID);
    }
}

uint32 LoadTexture(char* texturePath, bool repeat) {
    // TODO(Tobi): This is not how it was intended -.-
    // Also, this obviously leaks memory
    loaded_bitmap loadedBitmap = BitmapLoad(texturePath);

    if (loadedBitmap.Data) {
        uint32 ret;
        glGenTextures(1, &ret);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ret);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, loadedBitmap.Width, loadedBitmap.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, loadedBitmap.Data);
        return ret;
    } else {
        return OGLData.BrokenSprite.Sprite.TextureID; // TODO(Tobi): This is correct
    }
}

// TODO(Tobi): This is just copied 1:1 from below
sprite_data CreateSpriteFromBitmap(loaded_bitmap loadedBitmap, bool repeat) {
    sprite_data ret = {};

    if (loadedBitmap.Data) {
        glGenTextures(1, &ret.Sprite.TextureID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ret.Sprite.TextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, loadedBitmap.Width, loadedBitmap.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, loadedBitmap.Data);
    } else {
        return OGLData.BrokenSprite;
    }

    float left = 0;
    float right = (float) loadedBitmap.Width;
    float top = 0;
    float bottom = (float) loadedBitmap.Height;

    ret.Left = left;
    ret.Right = right;
    ret.Top = top;
    ret.Bottom = bottom;
    ret.LoadedBitmap = loadedBitmap;

    float vertices[] = {
        // Bound to the middle
        // positions             // texture coords
        right,    top, 0.0f,      1.0f, 1.0f, // top right
        right, bottom, 0.0f,      1.0f, 0.0f, // bottom right
         left, bottom, 0.0f,      0.0f, 0.0f, // bottom left
         left,    top, 0.0f,      0.0f, 1.0f  // top left 

        // Bound to the floor
        //  0.5f, 1.0f, 0.0f,      1.0f, 1.0f, // top right
        //  0.5f, 0.0f, 0.0f,      1.0f, 0.0f, // bottom right
        // -0.5f, 0.0f, 0.0f,      0.0f, 0.0f, // bottom left
        // -0.5f, 1.0f, 0.0f,      0.0f, 1.0f  // top left 
    };

    uint32 vbo;

    glGenVertexArrays(1, &ret.Sprite.VAO);
    glGenBuffers(1, &vbo);

    glBindVertexArray(ret.Sprite.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return ret;
}

sprite_data CreateSprite(char* texturePath, bool repeat) {
    sprite_data ret = {};

    //ret.TextureID = LoadTexture(texturePath);

    // Load texture
    loaded_bitmap loadedBitmap;
    {
        // TODO(Tobi): This is not how it was intended -.-
        // Also, this obviously leaks memory
        loadedBitmap = BitmapLoad(texturePath);

        if (loadedBitmap.Data) {
            glGenTextures(1, &ret.Sprite.TextureID);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ret.Sprite.TextureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, loadedBitmap.Width, loadedBitmap.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, loadedBitmap.Data);
        } else {
            return OGLData.BrokenSprite;
        }
    }
   
    // for now, I just make them as they are at the moment
    // float left = -0.5f;
    // float right = -0.5f + loadedBitmap.Width / 64.0f;
    // float top = -0.5f + loadedBitmap.Height / 64.0f;
    // float bottom = -0.5f;

    // float left = -loadedBitmap.Width / 2;
    // float right = -loadedBitmap.Width / 2 + loadedBitmap.Width;
    // float top = -loadedBitmap.Height / 2  + loadedBitmap.Height;
    // float bottom = -loadedBitmap.Height / 2;

    float left = 0;
    float right = (float) loadedBitmap.Width;
    float top = 0;
    float bottom = (float) loadedBitmap.Height;

    ret.Left = left;
    ret.Right = right;
    ret.Top = top;
    ret.Bottom = bottom;
    ret.LoadedBitmap = loadedBitmap;

    float vertices[] = {
        // Bound to the middle
        // positions             // texture coords
        right,    top, 0.0f,      1.0f, 1.0f, // top right
        right, bottom, 0.0f,      1.0f, 0.0f, // bottom right
         left, bottom, 0.0f,      0.0f, 0.0f, // bottom left
         left,    top, 0.0f,      0.0f, 1.0f  // top left 

        // Bound to the floor
        //  0.5f, 1.0f, 0.0f,      1.0f, 1.0f, // top right
        //  0.5f, 0.0f, 0.0f,      1.0f, 0.0f, // bottom right
        // -0.5f, 0.0f, 0.0f,      0.0f, 0.0f, // bottom left
        // -0.5f, 1.0f, 0.0f,      0.0f, 1.0f  // top left 
    };

    uint32 vbo;

    glGenVertexArrays(1, &ret.Sprite.VAO);
    glGenBuffers(1, &vbo);

    glBindVertexArray(ret.Sprite.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return ret;
}

// NOTE(Tobi): These are only supposed to be used while testing or in teh editor etc.

void ShaderResolveAndSetBool(int shaderID, char* varName, bool value) {
    GLCHK();

    int location = glGetUniformLocation(shaderID, varName);
    //Assert(location >= 0, "Location is %d", location);
    glUniform1i(location, (int) value);

    GLCHK();
}

void ShaderResolveAndSetInt(int shaderID, char* varName, int value) {
    GLCHK();

    int location = glGetUniformLocation(shaderID, varName);
    Assert(location >= 0, "Location is %d", location);
    glUniform1i(location, value);

    GLCHK();
}

void ShaderResolveAndSetFloat(int shaderID, char* varName, float value) {
    GLCHK();

    int location = glGetUniformLocation(shaderID, varName);
    Assert(location >= 0, "Location is %d", location);
    glUniform1f(location, value);

    GLCHK();
}

void ShaderResolveAndSetVec3(int shaderID, char* varName, hmm_vec3* value) {
    GLCHK();

    int location = glGetUniformLocation(shaderID, varName);
    Assert(location >= 0, "Location is %d", location);
    glUniform3fv(location, 1, value->Elements);

    GLCHK();
}

void ShaderResolveAndSetVec4(int shaderID, char* varName, hmm_vec4* value) {
    GLCHK();

    int location = glGetUniformLocation(shaderID, varName);
    Assert(location >= 0, "Location is %d", location);
    glUniform4fv(location, 1, value->Elements);

    GLCHK();
}

void ShaderResolveAndSetMat4(int shaderID, char* varName, hmm_mat4* value) {
    GLCHK();
    
    int location = glGetUniformLocation(shaderID, varName);
    Assert(location >= 0, "Location is %d", location);
    glUniformMatrix4fv(location, 1, false, (float*) value);

    GLCHK();
}

void RendererInit() {
    glClearColor(0.5f, 0.7f, 1.0f, 0.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenBuffers(1, &OGLData.UniformTime);
    glBindBuffer(GL_UNIFORM_BUFFER, OGLData.UniformTime);
    glBufferData(GL_UNIFORM_BUFFER, 1 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &OGLData.BlockIDMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, OGLData.BlockIDMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 1 * sizeof(hmm_mat4), NULL, GL_DYNAMIC_DRAW);
    //glBindBuffer(GL_UNIFORM_BUFFER, 0); // NOTE(Tobi): Necessary?
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, OGLData.BlockIDMatrices, 0, 1 * sizeof(hmm_mat4));

    GLCHK();

    OGLData.BrokenShader = LoadShader("shaders/BrokenShaderVert.vs", "shaders/BrokenShaderFrag.fs");
    OGLData.BrokenSprite = CreateSprite("assets/images/AlphaTest.bmp", true);
    OGLData.WhiteSprite = CreateSprite("assets/images/White.bmp", true);
    OGLData.TopLeftUnitSprite = CreateSprite("assets/images/White_11.bmp", true);

    OGLData.BasicShader = LoadShader("shaders/BasicVert.vs", "shaders/BasicFrag.fs");
    OGLData.BasicAlphaCutoffShader = LoadShader("shaders/BasicVert.vs", "shaders/BasicAlphaFrag.fs");
    OGLData.SimpleShader = LoadShader("shaders/SimpleVert.vs", "shaders/SimpleFrag.fs");
    OGLData.OutlineShader = LoadShader("shaders/BasicVert.vs", "shaders/OutlineFrag.fs");
    OGLData.DiscShader = LoadShader("shaders/BasicVert.vs", "shaders/DiscFrag.fs");
    OGLData.CircleShader = LoadShader("shaders/BasicVert.vs", "shaders/CircleFrag.fs");

    OGLData.BackgroundShader1 = LoadShader("shaders/BasicVert.vs", "shaders/BackgroundFragPart1.fs");
    OGLData.BackgroundShader2 = LoadShader("shaders/BasicVert.vs", "shaders/BackgroundFragPart2.fs");
    OGLData.GaussianBlurShader = LoadShader("shaders/BasicVert.vs", "shaders/GaussBlur.fs");
    OGLData.BumpToNormalShader = LoadShader("shaders/BasicVert.vs", "shaders/BumpToNormal.fs");

    OGLData.ManaBarShader = LoadShader("shaders/BasicVert.vs", "shaders/ManaBar.fs");

    GLCHK();

    float vertices[] = {
        // positions             // texture coords
        //  0.5f,  0.5f, 0.0f,      1.0f, 1.0f, // top right
        //  0.5f, -0.5f, 0.0f,      1.0f, 0.0f, // bottom right
        // -0.5f, -0.5f, 0.0f,      0.0f, 0.0f, // bottom left
        // -0.5f,  0.5f, 0.0f,      0.0f, 1.0f  // top left 

        //  0.5f, 1.0f, 0.0f,      1.0f, 1.0f, // top right
        //  0.5f, 0.0f, 0.0f,      1.0f, 0.0f, // bottom right
        // -0.5f, 0.0f, 0.0f,      0.0f, 0.0f, // bottom left
        // -0.5f, 1.0f, 0.0f,      0.0f, 1.0f  // top left 

        1.0f, 1.0f, 0.0f,      1.0f, 1.0f, // top right
        1.0f, 0.0f, 0.0f,      1.0f, 0.0f, // bottom right
        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, // bottom left
        0.0f, 1.0f, 0.0f,      0.0f, 1.0f  // top left 
    };

    // TODO(Tobi): Where do I use the vertices thing above and the stuff below

    uint32 vbo;

    glGenVertexArrays(1, &OGLData.DummyVAO);
    glGenBuffers(1, &vbo);

    glBindVertexArray(OGLData.DummyVAO);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // // NOTE(Tobi): I leave the stuff bound, since I only use this all the time

    // glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);

    // // NOTE(Tobi): At the moment I just use normalised sprites with their origin at their center
    // // (oh, that will be annoying when rendering things bigger than one tile)

    // // TODO(Tobi): What exactly do I do here?
    // // TODO(Tobi): When in debug mode, setup arrays for debug lines and shapes etc. (probably also debug IMGUI stuff (including letters))

    // // Now, I need some space to put shaders etc. to; also textures etc.


    // // Due to several reasons, I feel that the whole rendering thing is different in edit/debug mode and in run mode

    GLCHK();
}

void RendererStartFrame() {
    GLCHK();

    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    //glViewport(100, 100, 300, 300);

    // TODO(Tobi): Enable/Disable alpha stuff (at the moment, I think everything is alpha)

    // TODO(Tobi): Normally per sprite; but most things are like that for now

    // TODO(Tobi): I will only need that for smoke at the moment
    // TODO(Tobi): Provide hexes and triangles as ... hexes and triangles :D (Instead of rects with alpha cutoffs)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    /// Setup projection and view matrices
    {
        // NOTE(Tobi): ViewMatrix would be
        hmm_mat4 viewRotation = HMM_Mat4d(1.0f); //HMM_RotateRadians(HMM_PI32 / 4, HMM_Vec3(0, 0, 1));
        //hmm_mat4 viewTranslation = HMM_Translate(level->Camera.X, level->Camera.Y, -10);
        hmm_mat4 viewTranslation = HMM_Translate(0, 0, -10);
        hmm_mat4 viewScale = HMM_Mat4d(1.0f); //HMM_Scale(1, 1, 1);
        hmm_mat4 view = viewTranslation * viewRotation * viewScale; 
        // platform.WindowData->UserdefinedData.OGLContext->ViewMatrix = view;
        // Since I don't do any of these; I ignore that (TODO(Tobi): What about screenshake; that could be needed at one point)


        // NOTE(Tobi): This probably is almost always the same calculation
        //hmm_mat4 projection = HMM_Orthographic(0, OGLData.ActiveContext->ViewportWidth, 0, OGLData.ActiveContext->ViewportHeight, 0.1f, 100.0f);
        hmm_mat4 projection = HMM_Orthographic(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0.1f, 100.0f);
        hmm_mat4 projectionViewMatrix = projection * view;

        GLCHK();

        glBindBuffer(GL_UNIFORM_BUFFER, OGLData.BlockIDMatrices); // NOTE(Tobi): Is this necessary when doing the next thing?
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, OGLData.BlockIDMatrices, 0, sizeof(hmm_mat4));
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(hmm_mat4), &projectionViewMatrix);
        //glBindBuffer(GL_UNIFORM_BUFFER, 0); // NOTE(Tobi): Do I have to do that?

        GLCHK();

        // TODO(Tobi): Does this even need to be a variable since I calculate it everytime anyway?
        // TODO(Tobi): I don't want to use clock() I think
        // OGLData.RunningTime = clock()/(float) CLOCKS_PER_SEC;

        // glBindBuffer(GL_UNIFORM_BUFFER, OGLData.UniformTime);
        // glBindBufferRange(GL_UNIFORM_BUFFER, 0, OGLData.UniformTime, 0, sizeof(float));
        // glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &OGLData.RunningTime);

        //glBindBuffer(GL_UNIFORM_BUFFER, OGLData.BlockIDMatrices);

        GLCHK();
    }

    glEnable(GL_SCISSOR_TEST);

    glEnable(GL_DEPTH_TEST);

    GLCHK();
}

void RendererSetDrawRect(draw_rect* drawRect) {
    glScissor(drawRect->StartX, WINDOW_HEIGHT - drawRect->StartY - drawRect->Height, drawRect->Width, drawRect->Height);
}

#if 0
void RendererRender() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glViewport(100, 100, 300, 300);

    // TODO(Tobi): Enable/Disable alpha stuff (at the moment, I think everything is alpha)

    // TODO(Tobi): Normally per sprite; but most things are like that for now

    // TODO(Tobi): I will only need that for smoke at the moment
    // TODO(Tobi): Provide hexes and triangles as ... hexes and triangles :D (Instead of rects with alpha cutoffs)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    /// Setup projection and view matrices
    {
        // NOTE(Tobi): ViewMatrix would be
        hmm_mat4 viewRotation = HMM_Mat4d(1.0f); //HMM_RotateRadians(HMM_PI32 / 4, HMM_Vec3(0, 0, 1));
        //hmm_mat4 viewTranslation = HMM_Translate(level->Camera.X, level->Camera.Y, -10);
        hmm_mat4 viewTranslation = HMM_Translate(0, 0, -10);
        hmm_mat4 viewScale = HMM_Mat4d(1.0f); //HMM_Scale(1, 1, 1);
        hmm_mat4 view = viewTranslation * viewRotation * viewScale; 
        // platform.WindowData->UserdefinedData.OGLContext->ViewMatrix = view;
        // Since I don't do any of these; I ignore that (TODO(Tobi): What about screenshake; that could be needed at one point)


        // NOTE(Tobi): This probably is almost always the same calculation
        //hmm_mat4 projection = HMM_Orthographic(0, OGLData.ActiveContext->ViewportWidth, 0, OGLData.ActiveContext->ViewportHeight, 0.1f, 100.0f);
        hmm_mat4 projection = HMM_Orthographic(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0.1f, 100.0f);
        hmm_mat4 projectionViewMatrix = projection * view;

        glBindBuffer(GL_UNIFORM_BUFFER, OGLData.BlockIDMatrices); // NOTE(Tobi): Is this necessary when doing the next thing?
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, OGLData.BlockIDMatrices, 0, sizeof(hmm_mat4));
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(hmm_mat4), &projectionViewMatrix);
        glBindBuffer(GL_UNIFORM_BUFFER, 0); // NOTE(Tobi): Do I have to do that?
    }

    glEnable(GL_SCISSOR_TEST);

    // TODO(Tobi): I should enable depth testing; however, then I have to set the z-value better
    // glEnable(GL_DEPTH_TEST);

    /// Render sprites one-by-one
    inc0 (layer_i,   RENDER_LAYER_COUNT) {
        glScissor(OGLData.LayerDrawRects[layer_i].StartX, WINDOW_HEIGHT - OGLData.LayerDrawRects[layer_i].StartY - OGLData.LayerDrawRects[layer_i].Height, OGLData.LayerDrawRects[layer_i].Width, OGLData.LayerDrawRects[layer_i].Height);
        inc0 (renderObject_i,   OGLData.LayerSizes[layer_i]) {
            render_object* ro = &OGLData.LayerData[layer_i][renderObject_i];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ro->Sprite.TextureID);
            glUseProgram(ro->ShaderID);

            // TODO(Tobi): Resolve the stuff once; don't know where to save that though
            ShaderResolveAndSetMat4(ro->ShaderID, "Model", &ro->ModelMatrix);
            ShaderResolveAndSetVec4(ro->ShaderID, "Color", (hmm_vec4*) &ro->Color);
            
            glBindVertexArray(ro->Sprite.VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            //glBindVertexArray(0);
        }

        OGLData.LayerSizes[layer_i] = 0;
    }

    glDisable(GL_SCISSOR_TEST);

    // TODO(Tobi): Render debug stuff (lines, boxes, text, etc.)
    // TODO(Tobi): Is there really a need to separte them; can't I mix them together?
}
#endif 0

void RendererPushObject(render_object* ro) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ro->Sprite.TextureID);
    glUseProgram(ro->ShaderID);

    // TODO(Tobi): Resolve the stuff once; don't know where to save that though
    ShaderResolveAndSetMat4(ro->ShaderID, "Model", &ro->ModelMatrix);
    ShaderResolveAndSetVec4(ro->ShaderID, "Color", (hmm_vec4*) &ro->Color);
    
    glBindVertexArray(ro->Sprite.VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glBindVertexArray(0);
}

void RendererPushAlphaObject(render_object* ro) {
    glEnable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ro->Sprite.TextureID);
    glUseProgram(ro->ShaderID);

    // TODO(Tobi): Resolve the stuff once; don't know where to save that though
    ShaderResolveAndSetMat4(ro->ShaderID, "Model", &ro->ModelMatrix);
    ShaderResolveAndSetVec4(ro->ShaderID, "Color", (hmm_vec4*) &ro->Color);
    
    glBindVertexArray(ro->Sprite.VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glBindVertexArray(0);

    glDisable(GL_BLEND);
}

// TESTING
void RendererScreenSprite2(hmm_mat4 modelMatrix, sprite* spriteData1, sprite* spriteData2, uint32 shaderID) {
    glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
    glBindTexture(GL_TEXTURE_2D, spriteData1->TextureID);

    glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
    glBindTexture(GL_TEXTURE_2D, spriteData2->TextureID);

    glUseProgram(shaderID);

    // TODO(Tobi): Resolve the stuff once; don't know where to save that though

    ShaderResolveAndSetInt(shaderID, "texture1", 0);
    ShaderResolveAndSetInt(shaderID, "texture2", 1);

    ShaderResolveAndSetMat4(shaderID, "Model", &modelMatrix);
    color4f col = {1.0f, 1.0f, 1.0f, 1.0f};
    ShaderResolveAndSetVec4(shaderID, "Color", (hmm_vec4*) &col);
    
    glBindVertexArray(spriteData1->VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glBindVertexArray(0);
}

void RendererScreenSprite(hmm_mat4 modelMatrix, sprite* spriteData, uint32 shaderID, color4f col) {
    render_object ro = {};
    ro.ModelMatrix = modelMatrix;
    ro.Sprite = *spriteData;
    ro.ShaderID = shaderID;
    ro.Color = col;
    RendererPushObject(&ro);
}

void RendererScreenSpriteAlpha(hmm_mat4 modelMatrix, sprite* spriteData, uint32 shaderID, color4f col) {
    render_object ro = {};
    ro.ModelMatrix = modelMatrix;
    ro.Sprite = *spriteData;
    ro.ShaderID = shaderID;
    ro.Color = col;
    RendererPushAlphaObject(&ro);
}

void RendererScreenRect(hmm_mat4 modelMatrix, color4f col) {
    render_object ro = {};
    ro.ModelMatrix = modelMatrix;
    ro.Sprite = OGLData.TopLeftUnitSprite.Sprite;
    ro.ShaderID = OGLData.SimpleShader;
    ro.Color = col;
    RendererPushObject(&ro);
}

void RendererScreenRectAlpha(hmm_mat4 modelMatrix, color4f col) {
    render_object ro = {};
    ro.ModelMatrix = modelMatrix;
    ro.Sprite = OGLData.TopLeftUnitSprite.Sprite;
    ro.ShaderID = OGLData.SimpleShader;
    ro.Color = col;
    RendererPushAlphaObject(&ro);
}

void RendererScreenDisc(hmm_mat4 modelMatrix, color4f col) {
    render_object ro = {};
    ro.ModelMatrix = modelMatrix;
    ro.Sprite = OGLData.TopLeftUnitSprite.Sprite;
    ro.ShaderID = OGLData.DiscShader;
    ro.Color = col;
    RendererPushObject(&ro);
}

void RendererScreenCircle(hmm_mat4 modelMatrix, color4f col) {
    render_object ro = {};
    ro.ModelMatrix = modelMatrix;
    ro.Sprite = OGLData.TopLeftUnitSprite.Sprite;
    ro.ShaderID = OGLData.CircleShader;
    ro.Color = col;
    RendererPushObject(&ro);
}

// TODO(Tobi): Sprite datas is an array, but I only care about the VAO of the first one, maybe I should change it then
// NOTE(Tobi): VAO basically is the mesh of the quad/tri etc.; it's just that I decide to make them the same size
void RendererDrawAny(hmm_mat4 modelMatrix, sprite_data* spriteDatas, int spriteCount, uint32 shaderID, color4f colFloat) {
    Assert(spriteCount >= 1 && spriteCount <= 32, "Sprite count is an invalid number (%d)", spriteCount);

    char* textureNames[32] = {
        "texture1",
        "texture2",
        "texture3",
        "texture4",
        "texture5",
        "texture6",
        "texture7",
        "texture8",
        "texture9",
        "texture10",
        "texture11",
        "texture12",
        "texture13",
        "texture14",
        "texture15",
        "texture16",
        "texture17",
        "texture18",
        "texture19",
        "texture20",
        "texture21",
        "texture22",
        "texture23",
        "texture24",
        "texture25",
        "texture26",
        "texture27",
        "texture28",
        "texture29",
        "texture30",
        "texture31",
        "texture32",
    };


    glUseProgram(shaderID);
    inc0 (sprite_i,   spriteCount) {
        glActiveTexture(GL_TEXTURE0 + sprite_i);
        glBindTexture(GL_TEXTURE_2D, spriteDatas[sprite_i].Sprite.TextureID);

        ShaderResolveAndSetInt(shaderID, textureNames[sprite_i], sprite_i);
    }

    // TODO(Tobi): Resolve the stuff once; don't know where to save that though
    ShaderResolveAndSetMat4(shaderID, "Model", &modelMatrix);
    ShaderResolveAndSetVec4(shaderID, "Color", (hmm_vec4*) &colFloat);
    
    glBindVertexArray(spriteDatas[0].Sprite.VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void RendererDrawAnyDebugSize(hmm_mat4 modelMatrix, sprite_data* spriteDatas, int spriteCount, uint32 shaderID, color4f colFloat, float width, float height) {
    Assert(spriteCount >= 1 && spriteCount <= 32, "Sprite count is an invalid number (%d)", spriteCount);

    char* textureNames[32] = {
        "texture1",
        "texture2",
        "texture3",
        "texture4",
        "texture5",
        "texture6",
        "texture7",
        "texture8",
        "texture9",
        "texture10",
        "texture11",
        "texture12",
        "texture13",
        "texture14",
        "texture15",
        "texture16",
        "texture17",
        "texture18",
        "texture19",
        "texture20",
        "texture21",
        "texture22",
        "texture23",
        "texture24",
        "texture25",
        "texture26",
        "texture27",
        "texture28",
        "texture29",
        "texture30",
        "texture31",
        "texture32",
    };


    glUseProgram(shaderID);
    inc0 (sprite_i,   spriteCount) {
        glActiveTexture(GL_TEXTURE0 + sprite_i);
        glBindTexture(GL_TEXTURE_2D, spriteDatas[sprite_i].Sprite.TextureID);

        ShaderResolveAndSetInt(shaderID, textureNames[sprite_i], sprite_i);
    }

    // TODO(Tobi): Resolve the stuff once; don't know where to save that though
    ShaderResolveAndSetMat4(shaderID, "Model", &modelMatrix);
    ShaderResolveAndSetVec4(shaderID, "Color", (hmm_vec4*) &colFloat);
    
    uint32 vao;
    {
        float left = 0;
        float right = width;
        float top = 0;
        float bottom = height;

        float vertices[] = {
            // positions             // texture coords
            right,    top, 0.0f,      1.0f, 1.0f, // top right
            right, bottom, 0.0f,      1.0f, 0.0f, // bottom right
            left, bottom, 0.0f,      0.0f, 0.0f, // bottom left
            left,    top, 0.0f,      0.0f, 1.0f  // top left 
        };


        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        uint32 vbo;
        glGenBuffers(1, &vbo);

        //glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (0 * sizeof(float)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
