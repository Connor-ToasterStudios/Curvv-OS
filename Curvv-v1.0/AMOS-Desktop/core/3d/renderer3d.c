/**
 * AMOS Desktop OS - 3D Renderer Implementation
 * 
 * This file implements the 3D rendering system for AMOS Desktop OS.
 * The renderer is implemented in pure C with assembly optimizations
 * for maximum performance.
 */

#include "renderer3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// External assembly functions
extern void amos_asm_clear_zbuffer(float* z_buffer, int width, int height, float value);
extern void amos_asm_rasterize_triangle(
    amos_renderer3d_t* renderer,
    float* x0, float* y0, float* z0,
    float* x1, float* y1, float* z1,
    float* x2, float* y2, float* z2,
    amos_color_t color);
extern void amos_asm_transform_vertices(
    amos_vec4_t* dest, 
    const amos_vec3_t* src, 
    const amos_mat4_t* matrix,
    int count);
extern void amos_asm_vector_normalize(amos_vec3_t* dest, const amos_vec3_t* src);
extern void amos_asm_matrix_mul(
    amos_mat4_t* dest, 
    const amos_mat4_t* a, 
    const amos_mat4_t* b);

// Initialize 3D renderer
bool amos_renderer3d_init(
    amos_renderer3d_t* renderer,
    amos_framebuffer_t* target,
    uint32_t width,
    uint32_t height
) {
    if (!renderer || !target) {
        return false;
    }
    
    // Set initial values
    renderer->target = target;
    renderer->width = width;
    renderer->height = height;
    renderer->mode = AMOS_RENDER_FLAT;
    renderer->light_count = 0;
    renderer->cull_backface = true;
    renderer->z_test = true;
    
    // Initialize camera with default values
    amos_vec3_t pos = {0, 0, -5};
    amos_vec3_t target = {0, 0, 0};
    amos_vec3_t up = {0, 1, 0};
    amos_camera_init(&renderer->camera, &pos, &target, &up, 
                     3.14159f / 4.0f, (float)width / (float)height, 0.1f, 1000.0f);
    
    // Initialize world matrix to identity
    renderer->world_matrix = amos_mat4_identity();
    
    // Initialize material with default values
    renderer->material.ambient = (amos_vec3_t){0.2f, 0.2f, 0.2f};
    renderer->material.diffuse = (amos_vec3_t){0.8f, 0.8f, 0.8f};
    renderer->material.specular = (amos_vec3_t){1.0f, 1.0f, 1.0f};
    renderer->material.shininess = 32.0f;
    
    // Allocate Z-buffer
    size_t z_buffer_size = width * height * sizeof(float);
    renderer->z_buffer = (float*)malloc(z_buffer_size);
    if (!renderer->z_buffer) {
        return false;
    }
    
    // Initialize Z-buffer to far plane value
    amos_renderer3d_clear_zbuffer(renderer);
    
    return true;
}

// Clean up and release 3D renderer resources
void amos_renderer3d_cleanup(amos_renderer3d_t* renderer) {
    if (renderer) {
        if (renderer->z_buffer) {
            free(renderer->z_buffer);
            renderer->z_buffer = NULL;
        }
    }
}

// Clear the Z-buffer
void amos_renderer3d_clear_zbuffer(amos_renderer3d_t* renderer) {
    if (!renderer || !renderer->z_buffer) {
        return;
    }
    
    // Use the optimized assembly function to clear the Z-buffer
    amos_asm_clear_zbuffer(
        renderer->z_buffer, 
        renderer->width, 
        renderer->height, 
        renderer->camera.far_plane
    );
}

// Set the active camera
void amos_renderer3d_set_camera(amos_renderer3d_t* renderer, const amos_camera_t* camera) {
    if (!renderer || !camera) {
        return;
    }
    
    // Copy camera data
    renderer->camera = *camera;
    
    // Update camera matrices
    amos_camera_update_matrices(&renderer->camera);
}

// Set the rendering mode
void amos_renderer3d_set_mode(amos_renderer3d_t* renderer, amos_render_mode_t mode) {
    if (renderer) {
        renderer->mode = mode;
    }
}

// Set the active material
void amos_renderer3d_set_material(amos_renderer3d_t* renderer, const amos_material_t* material) {
    if (!renderer || !material) {
        return;
    }
    
    renderer->material = *material;
}

// Add a light to the renderer
int amos_renderer3d_add_light(amos_renderer3d_t* renderer, const amos_light_t* light) {
    if (!renderer || !light || renderer->light_count >= 8) {
        return -1;
    }
    
    int light_index = renderer->light_count;
    renderer->lights[light_index] = *light;
    renderer->light_count++;
    
    return light_index;
}

// Remove a light from the renderer
void amos_renderer3d_remove_light(amos_renderer3d_t* renderer, int index) {
    if (!renderer || index < 0 || index >= renderer->light_count) {
        return;
    }
    
    // Shift remaining lights down
    for (int i = index; i < renderer->light_count - 1; i++) {
        renderer->lights[i] = renderer->lights[i + 1];
    }
    
    renderer->light_count--;
}

// Set the world transformation matrix
void amos_renderer3d_set_world_matrix(amos_renderer3d_t* renderer, const amos_mat4_t* matrix) {
    if (!renderer || !matrix) {
        return;
    }
    
    renderer->world_matrix = *matrix;
}

// Enable or disable backface culling
void amos_renderer3d_set_backface_culling(amos_renderer3d_t* renderer, bool enable) {
    if (renderer) {
        renderer->cull_backface = enable;
    }
}

// Enable or disable Z-testing (depth testing)
void amos_renderer3d_set_depth_test(amos_renderer3d_t* renderer, bool enable) {
    if (renderer) {
        renderer->z_test = enable;
    }
}

// Render a mesh
void amos_renderer3d_render_mesh(
    amos_renderer3d_t* renderer,
    const amos_mesh_t* mesh,
    const amos_transform_t* transform
) {
    if (!renderer || !mesh || !transform || !mesh->vertices || !mesh->indices) {
        return;
    }
    
    // Calculate model-view-projection matrix
    amos_mat4_t model_matrix = transform->matrix;
    amos_mat4_t mvp_matrix;
    amos_mat4_t temp_matrix;
    
    // MVP = proj * view * model
    amos_asm_matrix_mul(&temp_matrix, &renderer->camera.view_matrix, &model_matrix);
    amos_asm_matrix_mul(&mvp_matrix, &renderer->camera.proj_matrix, &temp_matrix);
    
    // Transform vertices
    amos_vec4_t* transformed_vertices = (amos_vec4_t*)malloc(mesh->vertex_count * sizeof(amos_vec4_t));
    if (!transformed_vertices) {
        // Handle allocation failure
        return;
    }
    
    // Use assembly-optimized function to transform all vertices at once
    amos_asm_transform_vertices(
        transformed_vertices,
        (const amos_vec3_t*)mesh->vertices,
        &mvp_matrix,
        mesh->vertex_count
    );
    
    // Render triangles
    for (uint32_t i = 0; i < mesh->index_count; i += 3) {
        if (i + 2 >= mesh->index_count) {
            break;  // Incomplete triangle
        }
        
        uint32_t idx0 = mesh->indices[i];
        uint32_t idx1 = mesh->indices[i + 1];
        uint32_t idx2 = mesh->indices[i + 2];
        
        if (idx0 >= mesh->vertex_count || idx1 >= mesh->vertex_count || idx2 >= mesh->vertex_count) {
            continue;  // Invalid indices
        }
        
        // Create triangle from vertices
        amos_triangle_t triangle;
        triangle.vertices[0] = mesh->vertices[idx0];
        triangle.vertices[1] = mesh->vertices[idx1];
        triangle.vertices[2] = mesh->vertices[idx2];
        
        // Render the triangle
        amos_renderer3d_render_triangle(renderer, &triangle);
    }
    
    // Clean up
    free(transformed_vertices);
}

// Render a triangle
void amos_renderer3d_render_triangle(
    amos_renderer3d_t* renderer,
    const amos_triangle_t* triangle
) {
    if (!renderer || !triangle) {
        return;
    }
    
    // Extract triangle vertices
    const amos_vertex_t* v0 = &triangle->vertices[0];
    const amos_vertex_t* v1 = &triangle->vertices[1];
    const amos_vertex_t* v2 = &triangle->vertices[2];
    
    // Backface culling
    if (renderer->cull_backface) {
        // Calculate triangle normal
        amos_vec3_t edge1 = amos_vec3_sub(&v1->position, &v0->position);
        amos_vec3_t edge2 = amos_vec3_sub(&v2->position, &v0->position);
        amos_vec3_t normal = amos_vec3_cross(&edge1, &edge2);
        
        // Normalize normal
        amos_asm_vector_normalize(&normal, &normal);
        
        // Determine if triangle is facing away from camera
        amos_vec3_t view_dir = {0, 0, 1};  // In camera space, view direction is +Z
        float dot_product = amos_vec3_dot(&normal, &view_dir);
        
        if (dot_product <= 0) {
            return;  // Cull this triangle
        }
    }
    
    // Transform vertices to screen space
    float x0, y0, z0;
    float x1, y1, z1;
    float x2, y2, z2;
    
    // TODO: Implement proper perspective division and viewport transform
    // For now, just use a simplified approach
    
    // Determine color based on rendering mode
    amos_color_t color;
    
    switch (renderer->mode) {
        case AMOS_RENDER_WIREFRAME:
            color = amos_color_rgb(255, 255, 255);  // White wireframe
            // Render wireframe (edges only)
            // TODO: Implement actual wireframe rendering
            break;
            
        case AMOS_RENDER_FLAT:
        default:
            // Use the material's diffuse color
            color = amos_color_rgb(
                (uint8_t)(renderer->material.diffuse.x * 255),
                (uint8_t)(renderer->material.diffuse.y * 255),
                (uint8_t)(renderer->material.diffuse.z * 255)
            );
            
            // Use assembly-optimized rasterizer
            amos_asm_rasterize_triangle(
                renderer,
                &x0, &y0, &z0,
                &x1, &y1, &z1,
                &x2, &y2, &z2,
                color
            );
            break;
    }
}

/* Vector operations */

// Create a 3D vector
amos_vec3_t amos_vec3_create(float x, float y, float z) {
    amos_vec3_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

// Add two 3D vectors
amos_vec3_t amos_vec3_add(const amos_vec3_t* a, const amos_vec3_t* b) {
    amos_vec3_t result;
    result.x = a->x + b->x;
    result.y = a->y + b->y;
    result.z = a->z + b->z;
    return result;
}

// Subtract two 3D vectors
amos_vec3_t amos_vec3_sub(const amos_vec3_t* a, const amos_vec3_t* b) {
    amos_vec3_t result;
    result.x = a->x - b->x;
    result.y = a->y - b->y;
    result.z = a->z - b->z;
    return result;
}

// Multiply a 3D vector by a scalar
amos_vec3_t amos_vec3_mul(const amos_vec3_t* v, float scalar) {
    amos_vec3_t result;
    result.x = v->x * scalar;
    result.y = v->y * scalar;
    result.z = v->z * scalar;
    return result;
}

// Calculate dot product of two 3D vectors
float amos_vec3_dot(const amos_vec3_t* a, const amos_vec3_t* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Calculate cross product of two 3D vectors
amos_vec3_t amos_vec3_cross(const amos_vec3_t* a, const amos_vec3_t* b) {
    amos_vec3_t result;
    result.x = a->y * b->z - a->z * b->y;
    result.y = a->z * b->x - a->x * b->z;
    result.z = a->x * b->y - a->y * b->x;
    return result;
}

// Calculate length of a 3D vector
float amos_vec3_length(const amos_vec3_t* v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

// Normalize a 3D vector
amos_vec3_t amos_vec3_normalize(const amos_vec3_t* v) {
    amos_vec3_t result;
    amos_asm_vector_normalize(&result, v);
    return result;
}

/* Matrix operations */

// Create an identity matrix
amos_mat4_t amos_mat4_identity() {
    amos_mat4_t m;
    memset(&m, 0, sizeof(amos_mat4_t));
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;
    return m;
}

// Create a translation matrix
amos_mat4_t amos_mat4_translation(float x, float y, float z) {
    amos_mat4_t m = amos_mat4_identity();
    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;
    return m;
}

// Create a rotation matrix around the X axis
amos_mat4_t amos_mat4_rotation_x(float angle) {
    amos_mat4_t m = amos_mat4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    
    m.m[1][1] = c;
    m.m[1][2] = s;
    m.m[2][1] = -s;
    m.m[2][2] = c;
    
    return m;
}

// Create a rotation matrix around the Y axis
amos_mat4_t amos_mat4_rotation_y(float angle) {
    amos_mat4_t m = amos_mat4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    
    m.m[0][0] = c;
    m.m[0][2] = -s;
    m.m[2][0] = s;
    m.m[2][2] = c;
    
    return m;
}

// Create a rotation matrix around the Z axis
amos_mat4_t amos_mat4_rotation_z(float angle) {
    amos_mat4_t m = amos_mat4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    
    m.m[0][0] = c;
    m.m[0][1] = s;
    m.m[1][0] = -s;
    m.m[1][1] = c;
    
    return m;
}

// Create a scaling matrix
amos_mat4_t amos_mat4_scaling(float x, float y, float z) {
    amos_mat4_t m = amos_mat4_identity();
    m.m[0][0] = x;
    m.m[1][1] = y;
    m.m[2][2] = z;
    return m;
}

// Multiply two matrices
amos_mat4_t amos_mat4_mul(const amos_mat4_t* a, const amos_mat4_t* b) {
    amos_mat4_t result;
    amos_asm_matrix_mul(&result, a, b);
    return result;
}

// Transform a 3D vector by a matrix
amos_vec3_t amos_mat4_transform_vec3(const amos_mat4_t* m, const amos_vec3_t* v) {
    amos_vec4_t v4;
    amos_vec3_t result;
    
    // Transform the vector
    v4.x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
    v4.y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
    v4.z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];
    v4.w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];
    
    // Perform perspective division if w is not 1
    if (v4.w != 0.0f && v4.w != 1.0f) {
        float inv_w = 1.0f / v4.w;
        result.x = v4.x * inv_w;
        result.y = v4.y * inv_w;
        result.z = v4.z * inv_w;
    } else {
        result.x = v4.x;
        result.y = v4.y;
        result.z = v4.z;
    }
    
    return result;
}

// Create a view matrix (for camera)
amos_mat4_t amos_mat4_look_at(
    const amos_vec3_t* eye,
    const amos_vec3_t* target,
    const amos_vec3_t* up
) {
    amos_mat4_t m;
    
    // Calculate camera axes
    amos_vec3_t z_axis = amos_vec3_sub(eye, target);
    amos_asm_vector_normalize(&z_axis, &z_axis);
    
    amos_vec3_t x_axis = amos_vec3_cross(up, &z_axis);
    amos_asm_vector_normalize(&x_axis, &x_axis);
    
    amos_vec3_t y_axis = amos_vec3_cross(&z_axis, &x_axis);
    
    // Build matrix
    m.m[0][0] = x_axis.x;
    m.m[0][1] = y_axis.x;
    m.m[0][2] = z_axis.x;
    m.m[0][3] = 0.0f;
    
    m.m[1][0] = x_axis.y;
    m.m[1][1] = y_axis.y;
    m.m[1][2] = z_axis.y;
    m.m[1][3] = 0.0f;
    
    m.m[2][0] = x_axis.z;
    m.m[2][1] = y_axis.z;
    m.m[2][2] = z_axis.z;
    m.m[2][3] = 0.0f;
    
    m.m[3][0] = -amos_vec3_dot(&x_axis, eye);
    m.m[3][1] = -amos_vec3_dot(&y_axis, eye);
    m.m[3][2] = -amos_vec3_dot(&z_axis, eye);
    m.m[3][3] = 1.0f;
    
    return m;
}

// Create a perspective projection matrix
amos_mat4_t amos_mat4_perspective(
    float fov,
    float aspect_ratio,
    float near_plane,
    float far_plane
) {
    amos_mat4_t m;
    memset(&m, 0, sizeof(amos_mat4_t));
    
    float tan_half_fov = tanf(fov * 0.5f);
    float f = 1.0f / tan_half_fov;
    float range_inv = 1.0f / (near_plane - far_plane);
    
    m.m[0][0] = f / aspect_ratio;
    m.m[1][1] = f;
    m.m[2][2] = (near_plane + far_plane) * range_inv;
    m.m[2][3] = -1.0f;
    m.m[3][2] = near_plane * far_plane * range_inv * 2.0f;
    
    return m;
}

/* Helper functions */

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
) {
    if (!camera) {
        return;
    }
    
    camera->position = *position;
    camera->target = *target;
    camera->up = *up;
    camera->fov = fov;
    camera->aspect_ratio = aspect_ratio;
    camera->near_plane = near_plane;
    camera->far_plane = far_plane;
    
    // Update matrices
    amos_camera_update_matrices(camera);
}

// Update the camera matrices
void amos_camera_update_matrices(amos_camera_t* camera) {
    if (!camera) {
        return;
    }
    
    // Calculate view matrix
    camera->view_matrix = amos_mat4_look_at(
        &camera->position,
        &camera->target,
        &camera->up
    );
    
    // Calculate projection matrix
    camera->proj_matrix = amos_mat4_perspective(
        camera->fov,
        camera->aspect_ratio,
        camera->near_plane,
        camera->far_plane
    );
}

// Initialize a mesh
bool amos_mesh_init(
    amos_mesh_t* mesh,
    amos_vertex_t* vertices,
    uint32_t vertex_count,
    uint32_t* indices,
    uint32_t index_count
) {
    if (!mesh) {
        return false;
    }
    
    mesh->vertices = vertices;
    mesh->vertex_count = vertex_count;
    mesh->indices = indices;
    mesh->index_count = index_count;
    
    return true;
}

// Clean up a mesh
void amos_mesh_cleanup(amos_mesh_t* mesh) {
    if (mesh) {
        if (mesh->vertices) {
            free(mesh->vertices);
            mesh->vertices = NULL;
        }
        
        if (mesh->indices) {
            free(mesh->indices);
            mesh->indices = NULL;
        }
        
        mesh->vertex_count = 0;
        mesh->index_count = 0;
    }
}

// Create a cube mesh
bool amos_mesh_create_cube(amos_mesh_t* mesh, float size) {
    if (!mesh) {
        return false;
    }
    
    // Allocate vertices (8 corners)
    mesh->vertex_count = 8;
    mesh->vertices = (amos_vertex_t*)malloc(mesh->vertex_count * sizeof(amos_vertex_t));
    if (!mesh->vertices) {
        return false;
    }
    
    // Allocate indices (6 faces * 2 triangles per face * 3 indices per triangle)
    mesh->index_count = 36;
    mesh->indices = (uint32_t*)malloc(mesh->index_count * sizeof(uint32_t));
    if (!mesh->indices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
        return false;
    }
    
    // Set up cube corners (vertices)
    float half_size = size * 0.5f;
    
    // Front-bottom-left
    mesh->vertices[0].position = (amos_vec3_t){-half_size, -half_size, -half_size};
    mesh->vertices[0].normal = (amos_vec3_t){-1.0f, -1.0f, -1.0f};
    mesh->vertices[0].color = (amos_vec3_t){1.0f, 0.0f, 0.0f};  // Red
    mesh->vertices[0].uv = (amos_vec2_t){0.0f, 0.0f};
    
    // Front-bottom-right
    mesh->vertices[1].position = (amos_vec3_t){half_size, -half_size, -half_size};
    mesh->vertices[1].normal = (amos_vec3_t){1.0f, -1.0f, -1.0f};
    mesh->vertices[1].color = (amos_vec3_t){0.0f, 1.0f, 0.0f};  // Green
    mesh->vertices[1].uv = (amos_vec2_t){1.0f, 0.0f};
    
    // Front-top-right
    mesh->vertices[2].position = (amos_vec3_t){half_size, half_size, -half_size};
    mesh->vertices[2].normal = (amos_vec3_t){1.0f, 1.0f, -1.0f};
    mesh->vertices[2].color = (amos_vec3_t){0.0f, 0.0f, 1.0f};  // Blue
    mesh->vertices[2].uv = (amos_vec2_t){1.0f, 1.0f};
    
    // Front-top-left
    mesh->vertices[3].position = (amos_vec3_t){-half_size, half_size, -half_size};
    mesh->vertices[3].normal = (amos_vec3_t){-1.0f, 1.0f, -1.0f};
    mesh->vertices[3].color = (amos_vec3_t){1.0f, 1.0f, 0.0f};  // Yellow
    mesh->vertices[3].uv = (amos_vec2_t){0.0f, 1.0f};
    
    // Back-bottom-left
    mesh->vertices[4].position = (amos_vec3_t){-half_size, -half_size, half_size};
    mesh->vertices[4].normal = (amos_vec3_t){-1.0f, -1.0f, 1.0f};
    mesh->vertices[4].color = (amos_vec3_t){1.0f, 0.0f, 1.0f};  // Magenta
    mesh->vertices[4].uv = (amos_vec2_t){1.0f, 0.0f};
    
    // Back-bottom-right
    mesh->vertices[5].position = (amos_vec3_t){half_size, -half_size, half_size};
    mesh->vertices[5].normal = (amos_vec3_t){1.0f, -1.0f, 1.0f};
    mesh->vertices[5].color = (amos_vec3_t){0.0f, 1.0f, 1.0f};  // Cyan
    mesh->vertices[5].uv = (amos_vec2_t){0.0f, 0.0f};
    
    // Back-top-right
    mesh->vertices[6].position = (amos_vec3_t){half_size, half_size, half_size};
    mesh->vertices[6].normal = (amos_vec3_t){1.0f, 1.0f, 1.0f};
    mesh->vertices[6].color = (amos_vec3_t){1.0f, 1.0f, 1.0f};  // White
    mesh->vertices[6].uv = (amos_vec2_t){0.0f, 1.0f};
    
    // Back-top-left
    mesh->vertices[7].position = (amos_vec3_t){-half_size, half_size, half_size};
    mesh->vertices[7].normal = (amos_vec3_t){-1.0f, 1.0f, 1.0f};
    mesh->vertices[7].color = (amos_vec3_t){0.5f, 0.5f, 0.5f};  // Gray
    mesh->vertices[7].uv = (amos_vec2_t){1.0f, 1.0f};
    
    // Normalize vertex normals
    for (uint32_t i = 0; i < mesh->vertex_count; i++) {
        amos_asm_vector_normalize(&mesh->vertices[i].normal, &mesh->vertices[i].normal);
    }
    
    // Set up triangles (indices)
    // Front face
    mesh->indices[0] = 0; mesh->indices[1] = 1; mesh->indices[2] = 2;
    mesh->indices[3] = 0; mesh->indices[4] = 2; mesh->indices[5] = 3;
    
    // Right face
    mesh->indices[6] = 1; mesh->indices[7] = 5; mesh->indices[8] = 6;
    mesh->indices[9] = 1; mesh->indices[10] = 6; mesh->indices[11] = 2;
    
    // Back face
    mesh->indices[12] = 5; mesh->indices[13] = 4; mesh->indices[14] = 7;
    mesh->indices[15] = 5; mesh->indices[16] = 7; mesh->indices[17] = 6;
    
    // Left face
    mesh->indices[18] = 4; mesh->indices[19] = 0; mesh->indices[20] = 3;
    mesh->indices[21] = 4; mesh->indices[22] = 3; mesh->indices[23] = 7;
    
    // Top face
    mesh->indices[24] = 3; mesh->indices[25] = 2; mesh->indices[26] = 6;
    mesh->indices[27] = 3; mesh->indices[28] = 6; mesh->indices[29] = 7;
    
    // Bottom face
    mesh->indices[30] = 4; mesh->indices[31] = 5; mesh->indices[32] = 1;
    mesh->indices[33] = 4; mesh->indices[34] = 1; mesh->indices[35] = 0;
    
    return true;
}

// Create a sphere mesh
bool amos_mesh_create_sphere(amos_mesh_t* mesh, float radius, int slices, int stacks) {
    if (!mesh || slices < 3 || stacks < 2) {
        return false;
    }
    
    // Calculate number of vertices and indices
    uint32_t vertex_count = (slices + 1) * (stacks + 1);
    uint32_t index_count = slices * stacks * 6;  // 2 triangles per grid cell, 3 indices per triangle
    
    // Allocate memory
    mesh->vertices = (amos_vertex_t*)malloc(vertex_count * sizeof(amos_vertex_t));
    if (!mesh->vertices) {
        return false;
    }
    
    mesh->indices = (uint32_t*)malloc(index_count * sizeof(uint32_t));
    if (!mesh->indices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
        return false;
    }
    
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;
    
    // Generate vertices
    for (int stack = 0; stack <= stacks; stack++) {
        float phi = 3.14159f * (float)stack / (float)stacks;
        float sin_phi = sinf(phi);
        float cos_phi = cosf(phi);
        
        for (int slice = 0; slice <= slices; slice++) {
            float theta = 2.0f * 3.14159f * (float)slice / (float)slices;
            float sin_theta = sinf(theta);
            float cos_theta = cosf(theta);
            
            uint32_t vertex_index = stack * (slices + 1) + slice;
            
            // Calculate position
            mesh->vertices[vertex_index].position.x = radius * sin_phi * cos_theta;
            mesh->vertices[vertex_index].position.y = radius * cos_phi;
            mesh->vertices[vertex_index].position.z = radius * sin_phi * sin_theta;
            
            // Normal is just the normalized position for a sphere
            mesh->vertices[vertex_index].normal = mesh->vertices[vertex_index].position;
            amos_asm_vector_normalize(&mesh->vertices[vertex_index].normal, 
                                    &mesh->vertices[vertex_index].normal);
            
            // Set color based on normal
            mesh->vertices[vertex_index].color.x = (mesh->vertices[vertex_index].normal.x + 1.0f) * 0.5f;
            mesh->vertices[vertex_index].color.y = (mesh->vertices[vertex_index].normal.y + 1.0f) * 0.5f;
            mesh->vertices[vertex_index].color.z = (mesh->vertices[vertex_index].normal.z + 1.0f) * 0.5f;
            
            // Set texture coordinates
            mesh->vertices[vertex_index].uv.x = (float)slice / (float)slices;
            mesh->vertices[vertex_index].uv.y = (float)stack / (float)stacks;
        }
    }
    
    // Generate indices
    uint32_t index = 0;
    for (int stack = 0; stack < stacks; stack++) {
        for (int slice = 0; slice < slices; slice++) {
            // Get the indices of the quad's four corners
            uint32_t top_left = stack * (slices + 1) + slice;
            uint32_t top_right = top_left + 1;
            uint32_t bottom_left = (stack + 1) * (slices + 1) + slice;
            uint32_t bottom_right = bottom_left + 1;
            
            // First triangle (top-left, bottom-left, bottom-right)
            mesh->indices[index++] = top_left;
            mesh->indices[index++] = bottom_left;
            mesh->indices[index++] = bottom_right;
            
            // Second triangle (top-left, bottom-right, top-right)
            mesh->indices[index++] = top_left;
            mesh->indices[index++] = bottom_right;
            mesh->indices[index++] = top_right;
        }
    }
    
    return true;
}