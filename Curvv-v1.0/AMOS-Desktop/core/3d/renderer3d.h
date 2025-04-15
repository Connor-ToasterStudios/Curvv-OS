/**
 * AMOS Desktop OS - 3D Renderer
 * 
 * This file defines the 3D renderer for the AMOS Desktop OS,
 * providing a hardware-accelerated 3D rendering pipeline.
 */

#ifndef AMOS_RENDERER3D_H
#define AMOS_RENDERER3D_H

#include "../graphics/framebuffer.h"
#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct amos_renderer3d_t amos_renderer3d_t;
typedef struct amos_mesh_t amos_mesh_t;
typedef struct amos_material_t amos_material_t;
typedef struct amos_camera_t amos_camera_t;
typedef struct amos_light_t amos_light_t;
typedef struct amos_shader_program_t amos_shader_program_t;

// Vector and matrix types
typedef struct {
    float x, y;
} amos_vec2_t;

typedef struct {
    float x, y, z;
} amos_vec3_t;

typedef struct {
    float x, y, z, w;
} amos_vec4_t;

typedef struct {
    float m[4][4]; // Column-major (same as OpenGL)
} amos_mat4_t;

// Vertex structure
typedef struct {
    amos_vec3_t position;
    amos_vec3_t normal;
    amos_vec2_t texcoord;
    amos_vec4_t color;
} amos_vertex_t;

// Mesh structure
struct amos_mesh_t {
    amos_vertex_t* vertices;
    uint32_t* indices;
    int vertex_count;
    int index_count;
    amos_material_t* material;
};

// Material structure
struct amos_material_t {
    amos_vec4_t ambient;
    amos_vec4_t diffuse;
    amos_vec4_t specular;
    float shininess;
    amos_framebuffer_t* diffuse_texture;
    amos_shader_program_t* shader;
};

// Camera structure
struct amos_camera_t {
    amos_vec3_t position;
    amos_vec3_t target;
    amos_vec3_t up;
    float fov;
    float aspect;
    float near_clip;
    float far_clip;
    amos_mat4_t view_matrix;
    amos_mat4_t projection_matrix;
};

// Light types
typedef enum {
    AMOS_LIGHT_DIRECTIONAL,
    AMOS_LIGHT_POINT,
    AMOS_LIGHT_SPOT
} amos_light_type_t;

// Light structure
struct amos_light_t {
    amos_light_type_t type;
    amos_vec3_t position;
    amos_vec3_t direction;
    amos_vec4_t color;
    float intensity;
    float range;
    float spot_angle;
};

// Renderer structure
struct amos_renderer3d_t {
    int width;
    int height;
    amos_framebuffer_t* color_buffer;
    float* depth_buffer;
    
    amos_camera_t camera;
    
    amos_light_t lights[8];
    int light_count;
    
    amos_mat4_t model_matrix;
    amos_mat4_t view_matrix;
    amos_mat4_t projection_matrix;
    amos_mat4_t mvp_matrix;
    
    // Shader programs
    amos_shader_program_t* current_shader;
    amos_shader_program_t default_shader;
    
    // Render states
    bool depth_test_enabled;
    bool backface_culling_enabled;
    bool wireframe_mode;
};

/**
 * Initialize the 3D renderer
 * 
 * @param renderer Pointer to renderer structure
 * @param width Width of the render target
 * @param height Height of the render target
 * @return true if initialization was successful, false otherwise
 */
bool amos_renderer3d_init(amos_renderer3d_t* renderer, int width, int height);

/**
 * Clean up and release renderer resources
 * 
 * @param renderer Pointer to renderer structure
 */
void amos_renderer3d_cleanup(amos_renderer3d_t* renderer);

/**
 * Resize the render target
 * 
 * @param renderer Pointer to renderer structure
 * @param width New width
 * @param height New height
 * @return true if resize was successful, false otherwise
 */
bool amos_renderer3d_resize(amos_renderer3d_t* renderer, int width, int height);

/**
 * Clear the color and depth buffers
 * 
 * @param renderer Pointer to renderer structure
 * @param color Clear color
 */
void amos_renderer3d_clear(amos_renderer3d_t* renderer, amos_color_t color);

/**
 * Set the camera parameters
 * 
 * @param renderer Pointer to renderer structure
 * @param position Camera position
 * @param target Camera target (look-at point)
 * @param up Camera up vector
 * @param fov Field of view in degrees
 * @param aspect Aspect ratio (width/height)
 * @param near_clip Near clip plane distance
 * @param far_clip Far clip plane distance
 */
void amos_renderer3d_set_camera(
    amos_renderer3d_t* renderer,
    const amos_vec3_t* position,
    const amos_vec3_t* target,
    const amos_vec3_t* up,
    float fov,
    float aspect,
    float near_clip,
    float far_clip
);

/**
 * Add a light to the renderer
 * 
 * @param renderer Pointer to renderer structure
 * @param type Light type
 * @param position Light position (for point and spot lights)
 * @param direction Light direction (for directional and spot lights)
 * @param color Light color
 * @param intensity Light intensity
 * @param range Light range (for point and spot lights)
 * @param spot_angle Spot angle in degrees (for spot lights)
 * @return Light index, or -1 on failure
 */
int amos_renderer3d_add_light(
    amos_renderer3d_t* renderer,
    amos_light_type_t type,
    const amos_vec3_t* position,
    const amos_vec3_t* direction,
    const amos_vec4_t* color,
    float intensity,
    float range,
    float spot_angle
);

/**
 * Set the model matrix for the next draw call
 * 
 * @param renderer Pointer to renderer structure
 * @param model_matrix Model matrix
 */
void amos_renderer3d_set_model_matrix(
    amos_renderer3d_t* renderer,
    const amos_mat4_t* model_matrix
);

/**
 * Set the current shader program
 * 
 * @param renderer Pointer to renderer structure
 * @param shader Shader program to use
 */
void amos_renderer3d_set_shader(
    amos_renderer3d_t* renderer,
    amos_shader_program_t* shader
);

/**
 * Create a mesh
 * 
 * @param vertices Array of vertices
 * @param vertex_count Number of vertices
 * @param indices Array of indices
 * @param index_count Number of indices
 * @return Pointer to created mesh, or NULL on failure
 */
amos_mesh_t* amos_mesh_create(
    const amos_vertex_t* vertices,
    int vertex_count,
    const uint32_t* indices,
    int index_count
);

/**
 * Destroy a mesh and free its resources
 * 
 * @param mesh Pointer to mesh
 */
void amos_mesh_destroy(amos_mesh_t* mesh);

/**
 * Create a material
 * 
 * @param ambient Ambient color
 * @param diffuse Diffuse color
 * @param specular Specular color
 * @param shininess Shininess factor
 * @return Pointer to created material, or NULL on failure
 */
amos_material_t* amos_material_create(
    const amos_vec4_t* ambient,
    const amos_vec4_t* diffuse,
    const amos_vec4_t* specular,
    float shininess
);

/**
 * Set a diffuse texture for a material
 * 
 * @param material Pointer to material
 * @param texture Texture framebuffer
 */
void amos_material_set_texture(
    amos_material_t* material,
    amos_framebuffer_t* texture
);

/**
 * Destroy a material and free its resources
 * 
 * @param material Pointer to material
 */
void amos_material_destroy(amos_material_t* material);

/**
 * Render a mesh
 * 
 * @param renderer Pointer to renderer structure
 * @param mesh Pointer to mesh
 */
void amos_renderer3d_render_mesh(
    amos_renderer3d_t* renderer,
    const amos_mesh_t* mesh
);

/**
 * Get the output framebuffer
 * 
 * @param renderer Pointer to renderer structure
 * @return Pointer to framebuffer
 */
amos_framebuffer_t* amos_renderer3d_get_framebuffer(
    const amos_renderer3d_t* renderer
);

/**
 * Vector operations
 */
void amos_vec3_add(const amos_vec3_t* a, const amos_vec3_t* b, amos_vec3_t* result);
void amos_vec3_subtract(const amos_vec3_t* a, const amos_vec3_t* b, amos_vec3_t* result);
void amos_vec3_multiply(const amos_vec3_t* a, float scalar, amos_vec3_t* result);
void amos_vec3_normalize(const amos_vec3_t* v, amos_vec3_t* result);
float amos_vec3_dot(const amos_vec3_t* a, const amos_vec3_t* b);
void amos_vec3_cross(const amos_vec3_t* a, const amos_vec3_t* b, amos_vec3_t* result);
float amos_vec3_length(const amos_vec3_t* v);

/**
 * Matrix operations
 */
void amos_mat4_identity(amos_mat4_t* m);
void amos_mat4_multiply(const amos_mat4_t* a, const amos_mat4_t* b, amos_mat4_t* result);
void amos_mat4_translate(amos_mat4_t* m, float x, float y, float z);
void amos_mat4_rotate(amos_mat4_t* m, float angle, float x, float y, float z);
void amos_mat4_scale(amos_mat4_t* m, float x, float y, float z);
void amos_mat4_perspective(amos_mat4_t* m, float fov, float aspect, float near_clip, float far_clip);
void amos_mat4_look_at(amos_mat4_t* m, const amos_vec3_t* eye, const amos_vec3_t* center, const amos_vec3_t* up);
void amos_mat4_transform_vec3(const amos_mat4_t* m, const amos_vec3_t* v, amos_vec3_t* result);
void amos_mat4_transform_vec4(const amos_mat4_t* m, const amos_vec4_t* v, amos_vec4_t* result);

#endif /* AMOS_RENDERER3D_H */