/**
 * AMOS Desktop OS - 3D Renderer
 * 
 * This file defines the 3D rendering system for AMOS Desktop OS.
 * The 3D renderer is implemented in pure C for maximum efficiency and
 * compatibility with the AMOS kernel.
 */

#ifndef AMOS_RENDERER3D_H
#define AMOS_RENDERER3D_H

#include "../graphics/framebuffer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

/**
 * 2D Vector with x, y components
 */
typedef struct {
    float x;
    float y;
} amos_vec2_t;

/**
 * 3D Vector with x, y, z components
 */
typedef struct {
    float x;
    float y;
    float z;
} amos_vec3_t;

/**
 * 4D Vector with x, y, z, w components (for homogeneous coordinates)
 */
typedef struct {
    float x;
    float y;
    float z;
    float w;
} amos_vec4_t;

/**
 * 4x4 Matrix for transformations
 */
typedef struct {
    float m[4][4];
} amos_mat4_t;

/**
 * Vertex data structure
 */
typedef struct {
    amos_vec3_t position;  // Position in 3D space
    amos_vec3_t normal;    // Normal vector
    amos_vec3_t color;     // RGB color
    amos_vec2_t uv;        // Texture coordinates
} amos_vertex_t;

/**
 * Triangle structure
 */
typedef struct {
    amos_vertex_t vertices[3];  // Three vertices of the triangle
} amos_triangle_t;

/**
 * Mesh structure
 */
typedef struct {
    amos_vertex_t* vertices;  // Array of vertices
    uint32_t* indices;        // Array of indices
    uint32_t vertex_count;    // Number of vertices
    uint32_t index_count;     // Number of indices
} amos_mesh_t;

/**
 * Camera structure
 */
typedef struct {
    amos_vec3_t position;     // Camera position
    amos_vec3_t target;       // Look-at target
    amos_vec3_t up;           // Up vector
    float fov;                // Field of view (in radians)
    float aspect_ratio;       // Aspect ratio (width/height)
    float near_plane;         // Near clipping plane
    float far_plane;          // Far clipping plane
    amos_mat4_t view_matrix;  // View matrix
    amos_mat4_t proj_matrix;  // Projection matrix
} amos_camera_t;

/**
 * Rendering state
 */
typedef enum {
    AMOS_RENDER_WIREFRAME,
    AMOS_RENDER_FLAT,
    AMOS_RENDER_GOURAUD,
    AMOS_RENDER_PHONG
} amos_render_mode_t;

/**
 * Light types
 */
typedef enum {
    AMOS_LIGHT_AMBIENT,
    AMOS_LIGHT_DIRECTIONAL,
    AMOS_LIGHT_POINT,
    AMOS_LIGHT_SPOT
} amos_light_type_t;

/**
 * Light structure
 */
typedef struct {
    amos_light_type_t type;  // Light type
    amos_vec3_t position;    // Position (for point/spot lights)
    amos_vec3_t direction;   // Direction (for directional/spot lights)
    amos_vec3_t color;       // Light color
    float intensity;         // Light intensity
    float range;             // Light range (for point/spot lights)
    float inner_angle;       // Inner cone angle (for spot lights)
    float outer_angle;       // Outer cone angle (for spot lights)
} amos_light_t;

/**
 * Material structure
 */
typedef struct {
    amos_vec3_t ambient;     // Ambient color
    amos_vec3_t diffuse;     // Diffuse color
    amos_vec3_t specular;    // Specular color
    float shininess;         // Specular shininess/power
} amos_material_t;

/**
 * Transform structure
 */
typedef struct {
    amos_vec3_t position;    // Position
    amos_vec3_t rotation;    // Rotation (Euler angles)
    amos_vec3_t scale;       // Scale
    amos_mat4_t matrix;      // Combined transformation matrix
} amos_transform_t;

/**
 * 3D renderer structure
 */
typedef struct {
    amos_framebuffer_t* target;    // Target framebuffer
    float* z_buffer;              // Z-buffer (depth buffer)
    uint32_t width;               // Viewport width
    uint32_t height;              // Viewport height
    amos_camera_t camera;         // Active camera
    amos_render_mode_t mode;      // Rendering mode
    amos_light_t lights[8];       // Lights array
    uint32_t light_count;         // Number of active lights
    amos_material_t material;     // Active material
    amos_mat4_t world_matrix;     // World transformation matrix
    bool cull_backface;           // Whether to cull back-facing triangles
    bool z_test;                  // Whether to perform depth testing
} amos_renderer3d_t;

/**
 * Initialize the 3D renderer
 * 
 * @param renderer Pointer to renderer structure
 * @param target Target framebuffer
 * @param width Viewport width
 * @param height Viewport height
 * @return true if initialization successful, false otherwise
 */
bool amos_renderer3d_init(
    amos_renderer3d_t* renderer,
    amos_framebuffer_t* target,
    uint32_t width,
    uint32_t height
);

/**
 * Clean up and release 3D renderer resources
 * 
 * @param renderer Pointer to renderer structure
 */
void amos_renderer3d_cleanup(amos_renderer3d_t* renderer);

/**
 * Clear the Z-buffer
 * 
 * @param renderer Pointer to renderer structure
 */
void amos_renderer3d_clear_zbuffer(amos_renderer3d_t* renderer);

/**
 * Set the active camera
 * 
 * @param renderer Pointer to renderer structure
 * @param camera Pointer to camera structure
 */
void amos_renderer3d_set_camera(amos_renderer3d_t* renderer, const amos_camera_t* camera);

/**
 * Set the rendering mode
 * 
 * @param renderer Pointer to renderer structure
 * @param mode Rendering mode
 */
void amos_renderer3d_set_mode(amos_renderer3d_t* renderer, amos_render_mode_t mode);

/**
 * Set the active material
 * 
 * @param renderer Pointer to renderer structure
 * @param material Pointer to material structure
 */
void amos_renderer3d_set_material(amos_renderer3d_t* renderer, const amos_material_t* material);

/**
 * Add a light to the renderer
 * 
 * @param renderer Pointer to renderer structure
 * @param light Pointer to light structure
 * @return Light index, or -1 if no more lights can be added
 */
int amos_renderer3d_add_light(amos_renderer3d_t* renderer, const amos_light_t* light);

/**
 * Remove a light from the renderer
 * 
 * @param renderer Pointer to renderer structure
 * @param index Light index to remove
 */
void amos_renderer3d_remove_light(amos_renderer3d_t* renderer, int index);

/**
 * Set the world transformation matrix
 * 
 * @param renderer Pointer to renderer structure
 * @param matrix Pointer to matrix structure
 */
void amos_renderer3d_set_world_matrix(amos_renderer3d_t* renderer, const amos_mat4_t* matrix);

/**
 * Enable or disable backface culling
 * 
 * @param renderer Pointer to renderer structure
 * @param enable Whether to enable backface culling
 */
void amos_renderer3d_set_backface_culling(amos_renderer3d_t* renderer, bool enable);

/**
 * Enable or disable Z-testing (depth testing)
 * 
 * @param renderer Pointer to renderer structure
 * @param enable Whether to enable depth testing
 */
void amos_renderer3d_set_depth_test(amos_renderer3d_t* renderer, bool enable);

/**
 * Render a mesh
 * 
 * @param renderer Pointer to renderer structure
 * @param mesh Pointer to mesh structure
 * @param transform Pointer to transform structure
 */
void amos_renderer3d_render_mesh(
    amos_renderer3d_t* renderer,
    const amos_mesh_t* mesh,
    const amos_transform_t* transform
);

/**
 * Render a triangle
 * 
 * @param renderer Pointer to renderer structure
 * @param triangle Pointer to triangle structure
 */
void amos_renderer3d_render_triangle(
    amos_renderer3d_t* renderer,
    const amos_triangle_t* triangle
);

/**
 * Vector operations
 */

// Create a 3D vector
amos_vec3_t amos_vec3_create(float x, float y, float z);

// Add two 3D vectors
amos_vec3_t amos_vec3_add(const amos_vec3_t* a, const amos_vec3_t* b);

// Subtract two 3D vectors
amos_vec3_t amos_vec3_sub(const amos_vec3_t* a, const amos_vec3_t* b);

// Multiply a 3D vector by a scalar
amos_vec3_t amos_vec3_mul(const amos_vec3_t* v, float scalar);

// Calculate dot product of two 3D vectors
float amos_vec3_dot(const amos_vec3_t* a, const amos_vec3_t* b);

// Calculate cross product of two 3D vectors
amos_vec3_t amos_vec3_cross(const amos_vec3_t* a, const amos_vec3_t* b);

// Calculate length of a 3D vector
float amos_vec3_length(const amos_vec3_t* v);

// Normalize a 3D vector
amos_vec3_t amos_vec3_normalize(const amos_vec3_t* v);

/**
 * Matrix operations
 */

// Create an identity matrix
amos_mat4_t amos_mat4_identity();

// Create a translation matrix
amos_mat4_t amos_mat4_translation(float x, float y, float z);

// Create a rotation matrix around the X axis
amos_mat4_t amos_mat4_rotation_x(float angle);

// Create a rotation matrix around the Y axis
amos_mat4_t amos_mat4_rotation_y(float angle);

// Create a rotation matrix around the Z axis
amos_mat4_t amos_mat4_rotation_z(float angle);

// Create a scaling matrix
amos_mat4_t amos_mat4_scaling(float x, float y, float z);

// Multiply two matrices
amos_mat4_t amos_mat4_mul(const amos_mat4_t* a, const amos_mat4_t* b);

// Transform a 3D vector by a matrix
amos_vec3_t amos_mat4_transform_vec3(const amos_mat4_t* m, const amos_vec3_t* v);

// Create a view matrix (for camera)
amos_mat4_t amos_mat4_look_at(
    const amos_vec3_t* eye,
    const amos_vec3_t* target,
    const amos_vec3_t* up
);

// Create a perspective projection matrix
amos_mat4_t amos_mat4_perspective(
    float fov,
    float aspect_ratio,
    float near_plane,
    float far_plane
);

/**
 * Helper functions
 */

// Initialize a camera
void amos_camera_init(
    amos_camera_t* camera,
    const amos_vec3_t* position,
    const amos_vec3_t* target,
    const amos_vec3_t* up,
    float fov,
    float aspect_ratio,
    float near_plane,
    float far_plane
);

// Update the camera matrices
void amos_camera_update_matrices(amos_camera_t* camera);

// Initialize a mesh
bool amos_mesh_init(
    amos_mesh_t* mesh,
    amos_vertex_t* vertices,
    uint32_t vertex_count,
    uint32_t* indices,
    uint32_t index_count
);

// Clean up a mesh
void amos_mesh_cleanup(amos_mesh_t* mesh);

// Create a cube mesh
bool amos_mesh_create_cube(amos_mesh_t* mesh, float size);

// Create a sphere mesh
bool amos_mesh_create_sphere(amos_mesh_t* mesh, float radius, int slices, int stacks);

#endif /* AMOS_RENDERER3D_H */