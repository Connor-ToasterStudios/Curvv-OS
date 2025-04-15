/**
 * AMOS Desktop OS - Shader Demo
 * 
 * This file demonstrates the 3D rendering capabilities of AMOS Desktop OS
 * with advanced shaders implemented in C and optimized with assembly.
 */

#include "../core/3d/renderer3d.h"
#include "../core/3d/shaders.h"
#include "../core/graphics/framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Demo state
typedef struct {
    amos_renderer3d_t renderer;
    amos_mesh_t* cube_mesh;
    amos_mesh_t* sphere_mesh;
    amos_material_t* phong_material;
    amos_material_t* textured_material;
    amos_framebuffer_t* texture;
    amos_shader_program_t phong_shader;
    amos_shader_program_t textured_shader;
    
    float rotation_angle;
    
    // Uniforms
    amos_vec3_t light_position;
    amos_vec4_t light_color;
    float ambient_intensity;
    float diffuse_intensity;
    float specular_intensity;
    float shininess;
} demo_state_t;

// Phong shader varying data
typedef struct {
    amos_vec3_t position;    // Position in world space
    amos_vec3_t normal;      // Normal in world space
    amos_vec2_t texcoord;    // Texture coordinates
} phong_varying_t;

// Forward declarations
void phong_vertex_shader(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out);

void phong_fragment_shader(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out);

void textured_vertex_shader(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out);

void textured_fragment_shader(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out);

amos_mesh_t* create_cube();
amos_mesh_t* create_sphere(int subdivisions);
void create_procedural_texture(amos_framebuffer_t* fb);
void render_frame(demo_state_t* state, float dt);
void update_uniforms(demo_state_t* state, float dt);

int main() {
    printf("AMOS Desktop OS - 3D Shader Demo\n");
    printf("Demonstrating assembly-optimized shader pipeline\n");
    
    demo_state_t state;
    
    // Initialize renderer
    if (!amos_renderer3d_init(&state.renderer, 800, 600)) {
        printf("Failed to initialize renderer\n");
        return 1;
    }
    
    // Initialize demo state
    state.rotation_angle = 0.0f;
    
    // Initialize uniforms
    state.light_position.x = 5.0f;
    state.light_position.y = 5.0f;
    state.light_position.z = 5.0f;
    
    state.light_color.x = 1.0f;
    state.light_color.y = 1.0f;
    state.light_color.z = 1.0f;
    state.light_color.w = 1.0f;
    
    state.ambient_intensity = 0.1f;
    state.diffuse_intensity = 0.7f;
    state.specular_intensity = 0.5f;
    state.shininess = 32.0f;
    
    // Initialize phong shader
    if (!amos_shader_program_init(
            &state.phong_shader,
            "phong",
            phong_vertex_shader,
            phong_fragment_shader,
            sizeof(phong_varying_t))) {
        printf("Failed to initialize phong shader\n");
        return 1;
    }
    
    // Add phong shader uniforms
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "model_matrix",
        AMOS_UNIFORM_MAT4,
        &state.renderer.model_matrix,
        sizeof(amos_mat4_t));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "view_matrix",
        AMOS_UNIFORM_MAT4,
        &state.renderer.view_matrix,
        sizeof(amos_mat4_t));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "projection_matrix",
        AMOS_UNIFORM_MAT4,
        &state.renderer.projection_matrix,
        sizeof(amos_mat4_t));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "light_position",
        AMOS_UNIFORM_VEC3,
        &state.light_position,
        sizeof(amos_vec3_t));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "light_color",
        AMOS_UNIFORM_VEC4,
        &state.light_color,
        sizeof(amos_vec4_t));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "ambient_intensity",
        AMOS_UNIFORM_FLOAT,
        &state.ambient_intensity,
        sizeof(float));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "diffuse_intensity",
        AMOS_UNIFORM_FLOAT,
        &state.diffuse_intensity,
        sizeof(float));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "specular_intensity",
        AMOS_UNIFORM_FLOAT,
        &state.specular_intensity,
        sizeof(float));
    
    amos_shader_program_add_uniform(
        &state.phong_shader,
        "shininess",
        AMOS_UNIFORM_FLOAT,
        &state.shininess,
        sizeof(float));
    
    // Initialize textured shader
    if (!amos_shader_program_init(
            &state.textured_shader,
            "textured",
            textured_vertex_shader,
            textured_fragment_shader,
            sizeof(phong_varying_t))) {
        printf("Failed to initialize textured shader\n");
        return 1;
    }
    
    // Add textured shader uniforms (same as phong plus a texture)
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "model_matrix",
        AMOS_UNIFORM_MAT4,
        &state.renderer.model_matrix,
        sizeof(amos_mat4_t));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "view_matrix",
        AMOS_UNIFORM_MAT4,
        &state.renderer.view_matrix,
        sizeof(amos_mat4_t));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "projection_matrix",
        AMOS_UNIFORM_MAT4,
        &state.renderer.projection_matrix,
        sizeof(amos_mat4_t));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "light_position",
        AMOS_UNIFORM_VEC3,
        &state.light_position,
        sizeof(amos_vec3_t));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "light_color",
        AMOS_UNIFORM_VEC4,
        &state.light_color,
        sizeof(amos_vec4_t));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "ambient_intensity",
        AMOS_UNIFORM_FLOAT,
        &state.ambient_intensity,
        sizeof(float));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "diffuse_intensity",
        AMOS_UNIFORM_FLOAT,
        &state.diffuse_intensity,
        sizeof(float));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "specular_intensity",
        AMOS_UNIFORM_FLOAT,
        &state.specular_intensity,
        sizeof(float));
    
    amos_shader_program_add_uniform(
        &state.textured_shader,
        "shininess",
        AMOS_UNIFORM_FLOAT,
        &state.shininess,
        sizeof(float));
    
    // Create procedural texture
    state.texture = (amos_framebuffer_t*)malloc(sizeof(amos_framebuffer_t));
    if (!amos_fb_init(state.texture, 256, 256, 4)) {
        printf("Failed to initialize texture\n");
        return 1;
    }
    create_procedural_texture(state.texture);
    
    // Create materials
    amos_vec4_t ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    amos_vec4_t diffuse = {0.8f, 0.8f, 0.8f, 1.0f};
    amos_vec4_t specular = {1.0f, 1.0f, 1.0f, 1.0f};
    
    state.phong_material = amos_material_create(&ambient, &diffuse, &specular, 32.0f);
    state.phong_material->shader = &state.phong_shader;
    
    state.textured_material = amos_material_create(&ambient, &diffuse, &specular, 32.0f);
    state.textured_material->shader = &state.textured_shader;
    amos_material_set_texture(state.textured_material, state.texture);
    
    // Create meshes
    state.cube_mesh = create_cube();
    state.cube_mesh->material = state.phong_material;
    
    state.sphere_mesh = create_sphere(3);
    state.sphere_mesh->material = state.textured_material;
    
    // Setup camera
    amos_vec3_t camera_position = {0.0f, 0.0f, 5.0f};
    amos_vec3_t camera_target = {0.0f, 0.0f, 0.0f};
    amos_vec3_t camera_up = {0.0f, 1.0f, 0.0f};
    
    amos_renderer3d_set_camera(
        &state.renderer,
        &camera_position,
        &camera_target,
        &camera_up,
        45.0f,
        800.0f / 600.0f,
        0.1f,
        100.0f
    );
    
    // Add a light
    amos_renderer3d_add_light(
        &state.renderer,
        AMOS_LIGHT_POINT,
        &state.light_position,
        NULL,
        &state.light_color,
        1.0f,
        50.0f,
        0.0f
    );
    
    // Enable depth testing and backface culling
    state.renderer.depth_test_enabled = true;
    state.renderer.backface_culling_enabled = true;
    
    // Main loop
    int running = 1;
    clock_t last_time = clock();
    
    while (running) {
        // Calculate delta time
        clock_t current_time = clock();
        float dt = (float)(current_time - last_time) / CLOCKS_PER_SEC;
        last_time = current_time;
        
        // Update and render
        update_uniforms(&state, dt);
        render_frame(&state, dt);
        
        // In a real application, we would handle input events here
        
        // Exit after a certain time for the demo
        state.rotation_angle += dt;
        if (state.rotation_angle > 10.0f) {
            running = 0;
        }
    }
    
    // Cleanup
    amos_mesh_destroy(state.cube_mesh);
    amos_mesh_destroy(state.sphere_mesh);
    amos_material_destroy(state.phong_material);
    amos_material_destroy(state.textured_material);
    amos_fb_cleanup(state.texture);
    free(state.texture);
    amos_renderer3d_cleanup(&state.renderer);
    
    printf("Shader demo completed successfully\n");
    return 0;
}

// Phong vertex shader implementation
void phong_vertex_shader(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out
) {
    phong_varying_t* v_out = (phong_varying_t*)varying_out;
    
    // Get uniforms
    amos_uniform_t* model_matrix = amos_shader_program_get_uniform((amos_shader_program_t*)program, "model_matrix");
    amos_uniform_t* view_matrix = amos_shader_program_get_uniform((amos_shader_program_t*)program, "view_matrix");
    amos_uniform_t* projection_matrix = amos_shader_program_get_uniform((amos_shader_program_t*)program, "projection_matrix");
    
    if (!model_matrix || !view_matrix || !projection_matrix) {
        return;
    }
    
    // Transform position
    amos_vec4_t position = {vertex_in->position.x, vertex_in->position.y, vertex_in->position.z, 1.0f};
    amos_vec4_t position_world;
    amos_mat4_transform_vec4((amos_mat4_t*)model_matrix->data, &position, &position_world);
    
    amos_vec4_t position_view;
    amos_mat4_transform_vec4((amos_mat4_t*)view_matrix->data, &position_world, &position_view);
    
    amos_mat4_transform_vec4((amos_mat4_t*)projection_matrix->data, &position_view, position_out);
    
    // Transform normal
    amos_vec4_t normal = {vertex_in->normal.x, vertex_in->normal.y, vertex_in->normal.z, 0.0f};
    amos_vec4_t normal_world;
    amos_mat4_transform_vec4((amos_mat4_t*)model_matrix->data, &normal, &normal_world);
    
    // Output varying data
    v_out->position.x = position_world.x;
    v_out->position.y = position_world.y;
    v_out->position.z = position_world.z;
    
    v_out->normal.x = normal_world.x;
    v_out->normal.y = normal_world.y;
    v_out->normal.z = normal_world.z;
    
    v_out->texcoord = vertex_in->texcoord;
}

// Phong fragment shader implementation
void phong_fragment_shader(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out
) {
    const phong_varying_t* v_in = (const phong_varying_t*)varying_in;
    
    // Get uniforms
    amos_uniform_t* light_position = amos_shader_program_get_uniform((amos_shader_program_t*)program, "light_position");
    amos_uniform_t* light_color = amos_shader_program_get_uniform((amos_shader_program_t*)program, "light_color");
    amos_uniform_t* ambient_intensity = amos_shader_program_get_uniform((amos_shader_program_t*)program, "ambient_intensity");
    amos_uniform_t* diffuse_intensity = amos_shader_program_get_uniform((amos_shader_program_t*)program, "diffuse_intensity");
    amos_uniform_t* specular_intensity = amos_shader_program_get_uniform((amos_shader_program_t*)program, "specular_intensity");
    amos_uniform_t* shininess = amos_shader_program_get_uniform((amos_shader_program_t*)program, "shininess");
    
    if (!light_position || !light_color || !ambient_intensity || 
        !diffuse_intensity || !specular_intensity || !shininess) {
        // Missing uniforms, output red for debugging
        color_out->x = 1.0f;
        color_out->y = 0.0f;
        color_out->z = 0.0f;
        color_out->w = 1.0f;
        return;
    }
    
    // Normalize normal
    amos_vec3_t normal;
    amos_vec3_normalize(&v_in->normal, &normal);
    
    // Calculate light direction
    amos_vec3_t light_dir;
    amos_vec3_subtract((amos_vec3_t*)light_position->data, &v_in->position, &light_dir);
    amos_vec3_normalize(&light_dir, &light_dir);
    
    // Calculate view direction (camera position is at origin in view space)
    amos_vec3_t view_dir;
    view_dir.x = 0.0f - v_in->position.x;
    view_dir.y = 0.0f - v_in->position.y;
    view_dir.z = 0.0f - v_in->position.z;
    amos_vec3_normalize(&view_dir, &view_dir);
    
    // Calculate reflection direction
    float dot_nl = amos_vec3_dot(&normal, &light_dir);
    amos_vec3_t reflection;
    amos_vec3_multiply(&normal, 2.0f * dot_nl, &reflection);
    amos_vec3_subtract(&reflection, &light_dir, &reflection);
    amos_vec3_normalize(&reflection, &reflection);
    
    // Calculate lighting components
    amos_vec4_t* lc = (amos_vec4_t*)light_color->data;
    float ambient = *(float*)ambient_intensity->data;
    float diffuse = *(float*)diffuse_intensity->data * fmaxf(0.0f, dot_nl);
    float specular = *(float*)specular_intensity->data * powf(fmaxf(0.0f, amos_vec3_dot(&reflection, &view_dir)), *(float*)shininess->data);
    
    // Calculate final color
    color_out->x = ambient * lc->x + diffuse * lc->x + specular * lc->x;
    color_out->y = ambient * lc->y + diffuse * lc->y + specular * lc->y;
    color_out->z = ambient * lc->z + diffuse * lc->z + specular * lc->z;
    color_out->w = 1.0f;
}

// Textured vertex shader implementation (same as phong for this demo)
void textured_vertex_shader(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out
) {
    phong_vertex_shader(program, vertex_in, position_out, varying_out);
}

// Textured fragment shader implementation
void textured_fragment_shader(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out
) {
    const phong_varying_t* v_in = (const phong_varying_t*)varying_in;
    
    // Sample texture (simple nearest neighbor sampling)
    amos_material_t* material = (amos_material_t*)program->uniforms[program->uniform_count - 1].data;
    amos_framebuffer_t* texture = material->diffuse_texture;
    
    int x = (int)(v_in->texcoord.x * texture->width) % texture->width;
    int y = (int)(v_in->texcoord.y * texture->height) % texture->height;
    
    amos_color_t texel = amos_fb_get_pixel(texture, x, y);
    
    // Convert texel to float color
    amos_vec4_t base_color;
    base_color.x = ((texel >> 16) & 0xFF) / 255.0f;
    base_color.y = ((texel >> 8) & 0xFF) / 255.0f;
    base_color.z = (texel & 0xFF) / 255.0f;
    base_color.w = ((texel >> 24) & 0xFF) / 255.0f;
    
    // Apply lighting similar to phong shader
    amos_uniform_t* light_position = amos_shader_program_get_uniform((amos_shader_program_t*)program, "light_position");
    amos_uniform_t* light_color = amos_shader_program_get_uniform((amos_shader_program_t*)program, "light_color");
    amos_uniform_t* ambient_intensity = amos_shader_program_get_uniform((amos_shader_program_t*)program, "ambient_intensity");
    amos_uniform_t* diffuse_intensity = amos_shader_program_get_uniform((amos_shader_program_t*)program, "diffuse_intensity");
    
    if (!light_position || !light_color || !ambient_intensity || !diffuse_intensity) {
        // Missing uniforms, output texture color only
        *color_out = base_color;
        return;
    }
    
    // Normalize normal
    amos_vec3_t normal;
    amos_vec3_normalize(&v_in->normal, &normal);
    
    // Calculate light direction
    amos_vec3_t light_dir;
    amos_vec3_subtract((amos_vec3_t*)light_position->data, &v_in->position, &light_dir);
    amos_vec3_normalize(&light_dir, &light_dir);
    
    // Calculate diffuse factor
    float diffuse_factor = fmaxf(0.0f, amos_vec3_dot(&normal, &light_dir));
    
    // Calculate lighting components
    amos_vec4_t* lc = (amos_vec4_t*)light_color->data;
    float ambient = *(float*)ambient_intensity->data;
    float diffuse = *(float*)diffuse_intensity->data * diffuse_factor;
    
    // Calculate final color
    color_out->x = base_color.x * (ambient * lc->x + diffuse * lc->x);
    color_out->y = base_color.y * (ambient * lc->y + diffuse * lc->y);
    color_out->z = base_color.z * (ambient * lc->z + diffuse * lc->z);
    color_out->w = base_color.w;
}

// Create a cube mesh
amos_mesh_t* create_cube() {
    amos_mesh_t* mesh = (amos_mesh_t*)malloc(sizeof(amos_mesh_t));
    if (!mesh) return NULL;
    
    // 8 vertices for a cube
    mesh->vertex_count = 8;
    mesh->vertices = (amos_vertex_t*)malloc(mesh->vertex_count * sizeof(amos_vertex_t));
    
    // 36 indices for a cube (6 faces, 2 triangles per face, 3 indices per triangle)
    mesh->index_count = 36;
    mesh->indices = (uint32_t*)malloc(mesh->index_count * sizeof(uint32_t));
    
    if (!mesh->vertices || !mesh->indices) {
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->indices) free(mesh->indices);
        free(mesh);
        return NULL;
    }
    
    // Define cube vertices (corners of a 1x1x1 cube centered at origin)
    mesh->vertices[0].position = (amos_vec3_t){-0.5f, -0.5f, -0.5f};
    mesh->vertices[1].position = (amos_vec3_t){0.5f, -0.5f, -0.5f};
    mesh->vertices[2].position = (amos_vec3_t){0.5f, 0.5f, -0.5f};
    mesh->vertices[3].position = (amos_vec3_t){-0.5f, 0.5f, -0.5f};
    mesh->vertices[4].position = (amos_vec3_t){-0.5f, -0.5f, 0.5f};
    mesh->vertices[5].position = (amos_vec3_t){0.5f, -0.5f, 0.5f};
    mesh->vertices[6].position = (amos_vec3_t){0.5f, 0.5f, 0.5f};
    mesh->vertices[7].position = (amos_vec3_t){-0.5f, 0.5f, 0.5f};
    
    // Define normals (pointing outward from the cube)
    mesh->vertices[0].normal = (amos_vec3_t){-0.577f, -0.577f, -0.577f};
    mesh->vertices[1].normal = (amos_vec3_t){0.577f, -0.577f, -0.577f};
    mesh->vertices[2].normal = (amos_vec3_t){0.577f, 0.577f, -0.577f};
    mesh->vertices[3].normal = (amos_vec3_t){-0.577f, 0.577f, -0.577f};
    mesh->vertices[4].normal = (amos_vec3_t){-0.577f, -0.577f, 0.577f};
    mesh->vertices[5].normal = (amos_vec3_t){0.577f, -0.577f, 0.577f};
    mesh->vertices[6].normal = (amos_vec3_t){0.577f, 0.577f, 0.577f};
    mesh->vertices[7].normal = (amos_vec3_t){-0.577f, 0.577f, 0.577f};
    
    // Define texture coordinates
    mesh->vertices[0].texcoord = (amos_vec2_t){0.0f, 0.0f};
    mesh->vertices[1].texcoord = (amos_vec2_t){1.0f, 0.0f};
    mesh->vertices[2].texcoord = (amos_vec2_t){1.0f, 1.0f};
    mesh->vertices[3].texcoord = (amos_vec2_t){0.0f, 1.0f};
    mesh->vertices[4].texcoord = (amos_vec2_t){0.0f, 0.0f};
    mesh->vertices[5].texcoord = (amos_vec2_t){1.0f, 0.0f};
    mesh->vertices[6].texcoord = (amos_vec2_t){1.0f, 1.0f};
    mesh->vertices[7].texcoord = (amos_vec2_t){0.0f, 1.0f};
    
    // Define vertex colors
    mesh->vertices[0].color = (amos_vec4_t){1.0f, 0.0f, 0.0f, 1.0f};
    mesh->vertices[1].color = (amos_vec4_t){0.0f, 1.0f, 0.0f, 1.0f};
    mesh->vertices[2].color = (amos_vec4_t){0.0f, 0.0f, 1.0f, 1.0f};
    mesh->vertices[3].color = (amos_vec4_t){1.0f, 1.0f, 0.0f, 1.0f};
    mesh->vertices[4].color = (amos_vec4_t){1.0f, 0.0f, 1.0f, 1.0f};
    mesh->vertices[5].color = (amos_vec4_t){0.0f, 1.0f, 1.0f, 1.0f};
    mesh->vertices[6].color = (amos_vec4_t){1.0f, 1.0f, 1.0f, 1.0f};
    mesh->vertices[7].color = (amos_vec4_t){0.5f, 0.5f, 0.5f, 1.0f};
    
    // Define indices for all 6 faces (2 triangles per face)
    // Front face
    mesh->indices[0] = 0; mesh->indices[1] = 1; mesh->indices[2] = 2;
    mesh->indices[3] = 0; mesh->indices[4] = 2; mesh->indices[5] = 3;
    
    // Back face
    mesh->indices[6] = 4; mesh->indices[7] = 7; mesh->indices[8] = 6;
    mesh->indices[9] = 4; mesh->indices[10] = 6; mesh->indices[11] = 5;
    
    // Left face
    mesh->indices[12] = 0; mesh->indices[13] = 3; mesh->indices[14] = 7;
    mesh->indices[15] = 0; mesh->indices[16] = 7; mesh->indices[17] = 4;
    
    // Right face
    mesh->indices[18] = 1; mesh->indices[19] = 5; mesh->indices[20] = 6;
    mesh->indices[21] = 1; mesh->indices[22] = 6; mesh->indices[23] = 2;
    
    // Top face
    mesh->indices[24] = 3; mesh->indices[25] = 2; mesh->indices[26] = 6;
    mesh->indices[27] = 3; mesh->indices[28] = 6; mesh->indices[29] = 7;
    
    // Bottom face
    mesh->indices[30] = 0; mesh->indices[31] = 4; mesh->indices[32] = 5;
    mesh->indices[33] = 0; mesh->indices[34] = 5; mesh->indices[35] = 1;
    
    mesh->material = NULL;  // Will be set later
    
    return mesh;
}

// Create a sphere mesh using icosphere subdivision
amos_mesh_t* create_sphere(int subdivisions) {
    // Start with an icosahedron and subdivide
    // (Implementation simplified for brevity - would generate an icosphere)
    
    // For this demo, we'll just create a simple low-poly sphere
    amos_mesh_t* mesh = (amos_mesh_t*)malloc(sizeof(amos_mesh_t));
    if (!mesh) return NULL;
    
    // Very simplified sphere with just 8 vertices
    mesh->vertex_count = 8;
    mesh->vertices = (amos_vertex_t*)malloc(mesh->vertex_count * sizeof(amos_vertex_t));
    
    // 24 indices (8 triangles)
    mesh->index_count = 24;
    mesh->indices = (uint32_t*)malloc(mesh->index_count * sizeof(uint32_t));
    
    if (!mesh->vertices || !mesh->indices) {
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->indices) free(mesh->indices);
        free(mesh);
        return NULL;
    }
    
    // Define vertices
    mesh->vertices[0].position = (amos_vec3_t){0.0f, 1.0f, 0.0f};   // Top
    mesh->vertices[1].position = (amos_vec3_t){0.0f, -1.0f, 0.0f};  // Bottom
    mesh->vertices[2].position = (amos_vec3_t){1.0f, 0.0f, 0.0f};   // Right
    mesh->vertices[3].position = (amos_vec3_t){-1.0f, 0.0f, 0.0f};  // Left
    mesh->vertices[4].position = (amos_vec3_t){0.0f, 0.0f, 1.0f};   // Front
    mesh->vertices[5].position = (amos_vec3_t){0.0f, 0.0f, -1.0f};  // Back
    mesh->vertices[6].position = (amos_vec3_t){0.7f, 0.7f, 0.0f};   // Top-right
    mesh->vertices[7].position = (amos_vec3_t){-0.7f, 0.7f, 0.0f};  // Top-left
    
    // Normals (just use position for a unit sphere)
    for (int i = 0; i < mesh->vertex_count; i++) {
        amos_vec3_normalize(&mesh->vertices[i].position, &mesh->vertices[i].normal);
    }
    
    // Texture coordinates
    mesh->vertices[0].texcoord = (amos_vec2_t){0.5f, 0.0f};
    mesh->vertices[1].texcoord = (amos_vec2_t){0.5f, 1.0f};
    mesh->vertices[2].texcoord = (amos_vec2_t){1.0f, 0.5f};
    mesh->vertices[3].texcoord = (amos_vec2_t){0.0f, 0.5f};
    mesh->vertices[4].texcoord = (amos_vec2_t){0.5f, 0.5f};
    mesh->vertices[5].texcoord = (amos_vec2_t){0.5f, 0.5f};
    mesh->vertices[6].texcoord = (amos_vec2_t){0.75f, 0.25f};
    mesh->vertices[7].texcoord = (amos_vec2_t){0.25f, 0.25f};
    
    // Colors
    for (int i = 0; i < mesh->vertex_count; i++) {
        mesh->vertices[i].color = (amos_vec4_t){1.0f, 1.0f, 1.0f, 1.0f};
    }
    
    // Indices
    int idx = 0;
    // Top triangles
    mesh->indices[idx++] = 0; mesh->indices[idx++] = 7; mesh->indices[idx++] = 3;
    mesh->indices[idx++] = 0; mesh->indices[idx++] = 3; mesh->indices[idx++] = 4;
    mesh->indices[idx++] = 0; mesh->indices[idx++] = 4; mesh->indices[idx++] = 2;
    mesh->indices[idx++] = 0; mesh->indices[idx++] = 2; mesh->indices[idx++] = 6;
    
    // Bottom triangles
    mesh->indices[idx++] = 1; mesh->indices[idx++] = 3; mesh->indices[idx++] = 7;
    mesh->indices[idx++] = 1; mesh->indices[idx++] = 4; mesh->indices[idx++] = 3;
    mesh->indices[idx++] = 1; mesh->indices[idx++] = 2; mesh->indices[idx++] = 4;
    mesh->indices[idx++] = 1; mesh->indices[idx++] = 6; mesh->indices[idx++] = 2;
    
    mesh->material = NULL;  // Will be set later
    
    return mesh;
}

// Create a procedural texture
void create_procedural_texture(amos_framebuffer_t* fb) {
    if (!fb || !fb->initialized) return;
    
    // Create a checkerboard pattern
    int check_size = 32;
    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {
            int check_x = (x / check_size) % 2;
            int check_y = (y / check_size) % 2;
            
            amos_color_t color;
            if (check_x == check_y) {
                color = amos_color_rgb(200, 200, 200);
            } else {
                color = amos_color_rgb(50, 50, 50);
            }
            
            amos_fb_set_pixel(fb, x, y, color);
        }
    }
    
    // Add a border
    int border_width = 4;
    amos_color_t border_color = amos_color_rgb(255, 0, 0);
    
    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {
            if (x < border_width || x >= fb->width - border_width ||
                y < border_width || y >= fb->height - border_width) {
                amos_fb_set_pixel(fb, x, y, border_color);
            }
        }
    }
}

// Update uniform values
void update_uniforms(demo_state_t* state, float dt) {
    // Rotate light around the scene
    float light_speed = 1.0f;
    float light_radius = 5.0f;
    
    state->light_position.x = sinf(state->rotation_angle * light_speed) * light_radius;
    state->light_position.z = cosf(state->rotation_angle * light_speed) * light_radius;
    state->light_position.y = 3.0f + sinf(state->rotation_angle * 0.5f) * 2.0f;
    
    // Pulse the light color
    float t = (sinf(state->rotation_angle * 2.0f) + 1.0f) * 0.5f;
    state->light_color.x = 0.5f + 0.5f * t;
    state->light_color.y = 0.5f + 0.5f * (1.0f - t);
    state->light_color.z = 0.5f + 0.5f * sinf(state->rotation_angle * 4.0f);
}

// Render a single frame
void render_frame(demo_state_t* state, float dt) {
    // Clear the framebuffer
    amos_renderer3d_clear(&state->renderer, amos_color_rgb(10, 10, 40));
    
    // Set up model matrix for the cube
    amos_mat4_t model_matrix;
    amos_mat4_identity(&model_matrix);
    
    // Position and rotate the cube
    amos_mat4_translate(&model_matrix, -1.5f, 0.0f, 0.0f);
    amos_mat4_rotate(&model_matrix, state->rotation_angle, 0.0f, 1.0f, 0.0f);
    amos_mat4_rotate(&model_matrix, state->rotation_angle * 0.7f, 1.0f, 0.0f, 0.0f);
    amos_mat4_scale(&model_matrix, 1.0f, 1.0f, 1.0f);
    
    // Set model matrix and render cube
    amos_renderer3d_set_model_matrix(&state->renderer, &model_matrix);
    amos_renderer3d_render_mesh(&state->renderer, state->cube_mesh);
    
    // Set up model matrix for the sphere
    amos_mat4_identity(&model_matrix);
    
    // Position and rotate the sphere
    amos_mat4_translate(&model_matrix, 1.5f, 0.0f, 0.0f);
    amos_mat4_rotate(&model_matrix, state->rotation_angle * 0.5f, 0.0f, 1.0f, 0.0f);
    amos_mat4_scale(&model_matrix, 1.0f, 1.0f, 1.0f);
    
    // Set model matrix and render sphere
    amos_renderer3d_set_model_matrix(&state->renderer, &model_matrix);
    amos_renderer3d_render_mesh(&state->renderer, state->sphere_mesh);
}