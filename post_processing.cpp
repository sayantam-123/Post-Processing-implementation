#include <ctime>
#include <raylib.h>
#include <rlgl.h>
#include <glad/glad.h>
#include <raymath.h>
#include <cstdio>





struct GBuffer{
    unsigned int id;
    unsigned int colorTex;
    unsigned int depthTex;
};



void switch_shader(int& curr_shader);
GBuffer CreateGBuffer(int width, int height);
void switch_line(Shader& predator,int& scanLoc , int& scan);
void DrawFullscreenQuadRaw(unsigned int shaderID, unsigned int colorTex, unsigned int depthTex);
Matrix GetViewMatrix(Camera3D cam);
Matrix GetProjectionMatrix(Camera3D cam, float aspect);


enum shaderType{
    sobel,
    gray_lum,
    negative,
    predator,
    nothing,
    laplace,
    film_grain,
    motion_blur
};


int main(){
    int screenWidth = 1400;
    int screenHeight = 900;


    InitWindow(screenWidth,screenHeight,"bleh bleh bluh bluh" );

    Camera3D camera{.position = (Vector3){10.0f,10.0f,10.0f},.target = (Vector3){0.0f,0.0f,0.0f},.up = (Vector3){0.0f,1.0f,0.0f},.fovy = 45.0f,.projection = CAMERA_PERSPECTIVE};

    Vector3 cubePosition = {0.0f,15.0f,0.0f};

    Model model = LoadModel("clock-shaped-as-a-highlanders-cottage/source/1259/1259_MT_Zegar.obj");


    Shader shaders[10] = {0};

    Shader default_shader = LoadShader(NULL,NULL);
    shaders[0] = default_shader;


    //grayscale only luminance
    Shader gray_lum = LoadShader(NULL, "grayscale_lum.fs");
    shaders[1] = gray_lum;

    Shader negative = LoadShader(NULL, "negative.fs");
    shaders[2] = negative;

    Shader predator = LoadShader(NULL, "predator.fs");
    shaders[3] = predator;

    //sobel shader
    Shader sobel = LoadShader(NULL, "sobel.fs");
    shaders[4] = sobel;


    Shader laplace = LoadShader(NULL, "laplacian.fs");
    shaders[5] = laplace;

    //fim grain
    Shader filmgrain = LoadShader(NULL, "film_grain.fs");
    shaders[6] = filmgrain;

    int grainStrengthLoc = GetShaderLocation(filmgrain, "strength");
    float grainStrength = 0.2f;
    SetShaderValue(filmgrain, grainStrengthLoc, &grainStrength, SHADER_UNIFORM_FLOAT);

    int grainTimeLoc = GetShaderLocation(filmgrain, "time");



    int resLocL = GetShaderLocation(laplace, "resolution");
    float resL[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(laplace, resLocL, resL, SHADER_UNIFORM_VEC2);




    int resLoc = GetShaderLocation(sobel, "resolution");
    float res[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(sobel, resLoc, res, SHADER_UNIFORM_VEC2);


    int scanLoc = GetShaderLocation(predator, "show_scan");
    int show_scan = 0;
    SetShaderValue(predator, scanLoc, &show_scan, SHADER_UNIFORM_INT);
    int timeLoc = GetShaderLocation(predator, "time");


    int predResLoc = GetShaderLocation(predator, "resolution");
    SetShaderValue(predator, predResLoc, res, SHADER_UNIFORM_VEC2);

    RenderTexture2D offScreen = LoadRenderTexture(screenWidth,screenHeight );

    int current_shader = 0;


    //Just for motion blur part


    GBuffer gb = CreateGBuffer(screenWidth, screenHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // load a custom vertex shader too - needed for fullscreen triangle trick
    Shader motionBlur = LoadShader("mb.vs", "mb.fs");
    shaders[7] = motionBlur;
    int currVPInvLoc = GetShaderLocation(motionBlur, "currentVPInverse");
    int prevVPLoc_mb = GetShaderLocation(motionBlur, "previousVP");
    int samplesLoc   = GetShaderLocation(motionBlur, "numSamples");

    int samples = 6;            //increase this for strong motion blur
    SetShaderValue(motionBlur, samplesLoc, &samples, SHADER_UNIFORM_INT);

    float aspect = (float)screenWidth / (float)screenHeight;
    Matrix prevVP   = MatrixIdentity();
    bool first_frame = true;


    //dummyVAO
    unsigned int dummyVAO;
    glGenVertexArrays(1, &dummyVAO);


    //main
    DisableCursor();

    SetTargetFPS(60);

    while(!WindowShouldClose()){
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed(KEY_Z)) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };

        if (IsKeyPressed(KEY_F)) switch_shader(current_shader);

        if (current_shader == motion_blur) {
            Matrix view      = GetViewMatrix(camera);
            Matrix proj      = GetProjectionMatrix(camera, aspect);
            Matrix currentVP = MatrixMultiply(view, proj);
            Matrix vpInverse = MatrixInvert(currentVP);

            if(first_frame){ prevVP = currentVP; first_frame = false; }

            UpdateCamera(&camera, CAMERA_FREE);

            // --- Pass 1: render scene into GBuffer ---
            rlDrawRenderBatchActive();          // flush raylib before switching FB
            glBindFramebuffer(GL_FRAMEBUFFER, gb.id);
                glViewport(0, 0, screenWidth, screenHeight);
                glClearColor(255.0f/255, 255.0f/255, 200.0f/255, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);

                BeginMode3D(camera);
                    DrawModel(model, cubePosition, 0.1f, WHITE);
                    DrawGrid(20, 5.0f);
                EndMode3D();

            rlDrawRenderBatchActive();          // flush raylib's draw calls INTO gbuffer


            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);

            // --- Pass 2: motion blur ---
            BeginDrawing();
            ClearBackground(BLACK);

            // use shader directly via raw GL
            glUseProgram(motionBlur.id);

            // set matrix uniforms
            SetShaderValueMatrix(motionBlur, currVPInvLoc, vpInverse);
            SetShaderValueMatrix(motionBlur, prevVPLoc_mb, prevVP);
            SetShaderValue(motionBlur, samplesLoc, &samples, SHADER_UNIFORM_INT);

            // bind textures raw
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gb.colorTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gb.depthTex);

            // tell shader which slot each sampler uses
            glUniform1i(glGetUniformLocation(motionBlur.id, "texture1"), 0);
            glUniform1i(glGetUniformLocation(motionBlur.id, "texture2"), 1);

            // fullscreen triangle - no VAO needed with this trick
                glBindVertexArray(dummyVAO);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                glBindVertexArray(0);
            // cleanup
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(0);

            EndDrawing();

            prevVP = currentVP;
        }
        else{

            float t = GetTime();

            SetShaderValue(filmgrain, grainTimeLoc, &t, SHADER_UNIFORM_FLOAT);

            BeginTextureMode(offScreen);
                ClearBackground(RAYWHITE);

                BeginMode3D(camera);

                    DrawModel(model, cubePosition, 0.1f, WHITE );
                    DrawGrid(20, 5.0f);

                EndMode3D();

            EndTextureMode();

            BeginDrawing();

                ClearBackground(BLACK);

                BeginShaderMode(shaders[current_shader]);
                    DrawTextureRec(offScreen.texture, (Rectangle){0,0,(float)screenWidth,(float)-screenHeight},(Vector2){0,0},WHITE );
                EndShaderMode();
            EndDrawing();
        }

    }

    UnloadRenderTexture(offScreen);
    UnloadShader(sobel);
    CloseWindow();




    return 0;



}



Matrix GetViewMatrix(Camera3D cam){
    return MatrixLookAt(cam.position, cam.target, cam.up);
}

Matrix GetProjectionMatrix(Camera3D cam, float aspect){
    return MatrixPerspective(cam.fovy * DEG2RAD, aspect, 0.1f, 1000.0f);
}


void DrawFullscreenQuadRaw(unsigned int shaderID, unsigned int colorTex, unsigned int depthTex){
    // bind textures directly via raw GL
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    // set sampler uniforms directly
    glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderID, "texture2"), 1);

    // draw fullscreen triangle (no VAO needed - just emit verts)
    // use a simple fullscreen triangle trick
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // cleanup
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}




GBuffer CreateGBuffer(int width, int height){
    GBuffer gb;
    glGenFramebuffers(1, &gb.id);
    glBindFramebuffer(GL_FRAMEBUFFER, gb.id);

    glGenTextures(1, &gb.colorTex);
    glBindTexture(GL_TEXTURE_2D, gb.colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gb.colorTex, 0);

    glGenTextures(1, &gb.depthTex);
    glBindTexture(GL_TEXTURE_2D, gb.depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gb.depthTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return gb;
}



void switch_line(Shader& predator,int& scanLoc , int& scan){
    if(scan == 1){
        scan = 0;
        SetShaderValue(predator, scanLoc, &scan, SHADER_UNIFORM_INT);
    }
    else if (scan == 0) {
        scan = 1;
        SetShaderValue(predator, scanLoc, &scan, SHADER_UNIFORM_INT);
    }
}


void switch_shader(int& curr_shader){
    if (curr_shader == 7){
        curr_shader = 0;
    }else{
        curr_shader+=1;
    }
}
