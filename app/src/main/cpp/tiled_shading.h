#ifndef TIELD_SHADING
#define TIELD_SHADING

#define NEW_OBJ_LOADER 0

//extern int myPossibleLights = 1024;

#include <GLES3/gl3.h>


#include <stdlib.h>
#include <stdio.h>
#include <android_native_app_glue.h>
#include <jni.h>
#include "utils/Rendering.h"
#include "utils/SimpleCamera.h"
#include "utils/CheckGlError.h"
#include "utils/GlBufferObject.h"
#include "utils/Random.h"
#include "utils/SimpleShader.h"
#include "utils/ComboShader.h"

//#include <utils/PerformanceTimer.h>
//#include <utils/GLTimerQuery.h>

#include "OBJModel.h"
#include "Light.h"
#include "LightGrid.h"
#if NEW_OBJ_LOADER
#include "model_obj.h"
#endif

#include "external/libktx/include/ktx.h"
#include "Common/Include/texture_maps.h"
#include "Common/Include/common.h"
#include "Common/Include/bench.h"
//#include <glm/glm.hpp>




namespace chag
{
    typedef struct
    {
        //scene bounds
        int windowWidth;
        int windowHeight;
    } Info;

    enum RenderMethod
    {
        RM_TiledDeferred,
        RM_TiledForward,
        RM_Simple,
        RM_Max,
    };
    enum TiledDeferredUniformBufferSlots
    {
        TDUBS_Globals = OBJModel::UBS_Max, // use buffers that are not used by obj model.
        TDUBS_Max,
    };


    enum DeferredRenderTargetIndices
    {
        DRTI_Diffuse,
        DRTI_SpecularShininess,
        DRTI_Normal,
        DRTI_Ambient,
        DRTI_Depth,
        DRTI_Max,
    };

    enum TiledDeferredTextureUnits
    {
        TDTU_LightIndexData = OBJModel::TU_Max, // avoid those used by objmodel, will make life easier for tiled forward
        TDTU_Diffuse,
        TDTU_SpecularShininess,
        TDTU_Normal,
        TDTU_Ambient,
        TDTU_Depth,

        TDTU_LightGridCountOffsetData,
        //TDTU_LightColors,
        TDTU_LightPositionRanges3,
        TDTU_LightColors3,
        TDTU_Max,

    };

#if 1
    enum RenderFlags2
    {
        RF_Opaque2 = 1, /**< Draw chunks that are fully opaque, i.e. do not have transparency or an opacity map */
        RF_AlphaTested2 = 1 << 1,  /**< Draw chunks that have an opacity map */
        RF_Transparent2 = 1 << 2,  /**< Draw chunks that have an alpha value != 1.0f */
        RF_All2 = RF_Opaque2 | RF_AlphaTested2 | RF_Transparent2,  /**< Draw everything. */
    };
#endif

    class tiled_shading_app : public Bench
    {
    public:
        tiled_shading_app(int width, int height, int num, float x, float y);
        virtual ~tiled_shading_app();

        virtual void Draw() override;
        virtual bool Update() override;
        bool startup();
        virtual void Shutdown() override;
        virtual void SetLights(int num) override;
        int myPossibleLights;
        virtual void changeViews(float x, float y);
        float xPos;
        float yPos;

        Info info;


        glm::vec3 hueToRGB(float hue);
        void renderTiledDeferred(LightGrid &grid, const glm::mat4x4 &projectionMatrix);
        void updateShaderGlobals(const glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix, const glm::vec3 &worldUpDirection, int width, int height);
        void drawPreZPass();
        std::vector<Light> generateLights(const Aabb &aabb, int num);

        // helper function to create and attach a frame buffer target object.
        GLuint attachTargetTextureToFBO(GLuint fbo, GLenum attachment, int width, int height, GLenum internalFormat, GLenum format, GLenum type = GL_FLOAT, int msaaSamples = 0);

        void checkFBO(uint32_t fbo);
        void deleteTextureIfUsed(GLuint texId);
        void createFbos(int width, int height);
        void bindObjModelAttributes(ComboShader *shader);

        void setObjModelUniformBindings(ComboShader *shader);
        void setTiledLightingUniformBindings(ComboShader *shader);
        template <typename T> void deleteIfThere(T *&shader);
        void createShaders();

        // helper to bind texture...
        void bindTexture(GLenum type, int texUnit, int textureId);
        void bindLightGridConstants(const LightGrid &lightGrid);
        void downSampleDepthBuffer(std::vector<glm::vec2> &depthRanges);


        JNIEnv* jni;
        android_app* app_;
        JavaVM* javaVM;

        void updateFPS(float fFPS);
#if NEW_OBJ_LOADER
        void render_model(bool alphaTested); //uint32_t renderFlags);
        void init_buffers();
#endif

    private:



        OBJModel *g_model;
#if NEW_OBJ_LOADER
        ModelOBJ* g_model2;
#endif


        float g_near = 0.1f;
        float g_far = 10000.0f;
        const float g_fov = 45.0f;
        const glm::vec3 g_ambientLight = { 0.05f, 0.05f, 0.05f };

        LightGrid g_lightGrid;
        std::vector<Light> g_lights;
        SimpleCamera g_camera;

        ComboShader *g_simpleShader = 0;
        ComboShader *g_deferredShader = 0;
        ComboShader *g_tiledDeferredShader = 0;
        ComboShader *g_tiledForwardShader = 0;
        SimpleShader *g_downSampleMinMaxShader = 0;

        // Note: GL_RGBA32F and 16x aa doesn't appear to be supported on less than Fermi,
        //       Not tested on high-end AMD cards.
        //GLuint g_rgbaFpFormat = GL_RGBA8;//GL_RGBA16F;// GL_RGBA32F;// GL_R11F_G11F_B10F;//

        bool g_showLights = false;
        bool g_showLightGrid = false;
        int g_showGBuffer = 0;
        bool g_showInfo = false;
        bool g_enablePreZ =  true;//false;
        bool g_enableDepthRangeTest = true;

        uint32_t g_numMsaaSamples = 0;
#ifdef  MAX_ALLOWED_MSAA_SAMPLES
        uint32_t g_maxMsaaSamples = MAX_ALLOWED_MSAA_SAMPLES;
#else
        uint32_t g_maxMsaaSamples = 0;
#endif

        //std::string g_sceneFileName = "../scenes/house.obj";

        RenderMethod g_renderMethod = RM_TiledDeferred;// RM_TiledForward;// RM_TiledForward;// Simple;// RM_TiledDeferred;


        // deferred render target.
        GLuint g_deferredFbo = 0;
        GLuint g_renderTargetTextures[DRTI_Max];// = { 0 };
        //GLuint g_depthTargetTexture = 0;
        // forward MSAA render target (depth target is shared with deferred).
        GLuint g_forwardFbo = 0;
        GLuint g_forwardTargetTexture = 0;
        // Contains depth min/max per tile, downsampled from frame buffer
        GLuint g_minMaxDepthFbo = 0;
        GLuint g_minMaxDepthTargetTexture = 0;


        //vao for a render quad
        GLuint quad_vao;
        GLuint vbuffer;

        //vao for models
        GLuint g_vao;


        // Textures for grid and light data, bound to uniform blocks
        GLuint g_lightGridCountOffsetsTexture = 0;

        glm::ivec2 g_gridBuffer3[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];
        //glm::mediump_u16vec2 g_gridBuffer3[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];
        //glm::mediump_i16vec4 g_gridBuffer3[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];
        //glm::ivec4 g_gridBuffer3[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];
        //glm::vec4 g_gridBuffer3[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];

        GLuint g_lightPositionRangeTexture3 = 0;
        glm::vec4 g_lightPositionRangeBuffer3[NUM_POSSIBLE_LIGHTS];

        GLuint g_lightColorTexture = 0;
        GlBufferObject<glm::vec4> g_lightColorBuffer2;

        GLuint g_lightColorTexture3 = 0;
        glm::vec3 g_lightColorBuffer3[NUM_POSSIBLE_LIGHTS];


        // Texture that contains all the tile light lists.
        GLuint g_tileLightIndexListsTexture = 0;
        int g_tileLightIndexListsBuffer[65536];

        int g_oldFBO;


        struct ShaderGlobals_Std140
        {
            glm::mat4x4 viewMatrix;
            glm::mat4x4 viewProjectionMatrix;
            glm::mat4x4 inverseProjectionMatrix;
            glm::mat4x4 normalMatrix;
            glm::vec3 worldUpDirection;
            int bAlpha;
            //float pad0;
            glm::vec3 ambientGlobal;
            float pad1;
            glm::vec2 invFbSize;
            glm::ivec2 fbSize;
        } g_shaderGlobals;


        GlBufferObject<ShaderGlobals_Std140> g_shaderGlobalsGl;

        glm::mat4x4 projection;
        glm::mat4x4 modelView;

#if NEW_OBJ_LOADER
        GLuint g_positionVBO;
        GLuint g_indicesIBO;
        GLuint g_normalVBO;
        GLuint g_texCoordVBO;
        GLuint g_tangentsVBO;
        GLuint g_bitangentsVBO;

        // maps to layout of uniforms under std140 layout.
        struct MaterialProperties_Std140
        {
            glm::vec3 diffuse_color;
            float pad0;
            glm::vec3 specular_color;
            float pad1;
            glm::vec3 emissive_color;
            float specular_exponent;
            // this meets the alignment required for uniform buffer offsets on NVidia GTX280/480, also
            // compatible with AMD integrated Radeon HD 3100.
            float alignPad[52];
        };

        GlBufferObject<MaterialProperties_Std140> m_materialPropertiesBuffer;
        TextureMaps *m_diffuseMaps;
        TextureMaps *m_opacityMaps;
        TextureMaps *m_specularMaps;
        TextureMaps *m_normalMaps;
#endif
    };

}

#endif
