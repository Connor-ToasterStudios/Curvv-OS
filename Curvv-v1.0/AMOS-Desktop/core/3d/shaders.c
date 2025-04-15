/**
 * AMOS Desktop OS - 3D Renderer Shader System Implementation
 * 
 * This file implements the shader system for the AMOS 3D renderer,
 * providing a programmable vertex and fragment processing pipeline.
 */

#include "shaders.h"
#include <stdlib.h>
#include <string.h>

// Initialize a shader program
bool amos_shader_program_init(
    amos_shader_program_t* program,
    const char* name,
    amos_vertex_shader_fn vertex_shader,
    amos_fragment_shader_fn fragment_shader,
    int varying_size
) {
    if (!program || !name || !vertex_shader || !fragment_shader || varying_size < 0) {
        return false;
    }
    
    // Set name
    strncpy(program->name, name, AMOS_MAX_SHADER_NAME_LENGTH - 1);
    program->name[AMOS_MAX_SHADER_NAME_LENGTH - 1] = '\0';  // Ensure null termination
    
    // Set shader functions
    program->vertex_shader = vertex_shader;
    program->fragment_shader = fragment_shader;
    
    // Set varying size
    program->varying_size = varying_size;
    
    // Initialize uniform and attribute counts
    program->uniform_count = 0;
    program->attribute_count = 0;
    
    return true;
}

// Add a uniform to a shader program
bool amos_shader_program_add_uniform(
    amos_shader_program_t* program,
    const char* name,
    amos_uniform_type_t type,
    void* data,
    int size
) {
    if (!program || !name || !data || size <= 0) {
        return false;
    }
    
    // Check if we have room for another uniform
    if (program->uniform_count >= AMOS_MAX_UNIFORMS) {
        return false;
    }
    
    // Check if uniform already exists
    for (int i = 0; i < program->uniform_count; i++) {
        if (strcmp(program->uniforms[i].name, name) == 0) {
            // Update existing uniform
            program->uniforms[i].type = type;
            program->uniforms[i].data = data;
            program->uniforms[i].size = size;
            return true;
        }
    }
    
    // Add new uniform
    amos_uniform_t* uniform = &program->uniforms[program->uniform_count];
    strncpy(uniform->name, name, AMOS_MAX_SHADER_NAME_LENGTH - 1);
    uniform->name[AMOS_MAX_SHADER_NAME_LENGTH - 1] = '\0';  // Ensure null termination
    uniform->type = type;
    uniform->data = data;
    uniform->size = size;
    
    program->uniform_count++;
    
    return true;
}

// Add an attribute to a shader program
bool amos_shader_program_add_attribute(
    amos_shader_program_t* program,
    const char* name,
    amos_attribute_type_t type,
    int offset,
    int size
) {
    if (!program || !name || offset < 0 || size <= 0) {
        return false;
    }
    
    // Check if we have room for another attribute
    if (program->attribute_count >= AMOS_MAX_ATTRIBUTES) {
        return false;
    }
    
    // Check if attribute already exists
    for (int i = 0; i < program->attribute_count; i++) {
        if (strcmp(program->attributes[i].name, name) == 0) {
            // Update existing attribute
            program->attributes[i].type = type;
            program->attributes[i].offset = offset;
            program->attributes[i].size = size;
            return true;
        }
    }
    
    // Add new attribute
    amos_attribute_t* attribute = &program->attributes[program->attribute_count];
    strncpy(attribute->name, name, AMOS_MAX_SHADER_NAME_LENGTH - 1);
    attribute->name[AMOS_MAX_SHADER_NAME_LENGTH - 1] = '\0';  // Ensure null termination
    attribute->type = type;
    attribute->offset = offset;
    attribute->size = size;
    
    program->attribute_count++;
    
    return true;
}

// Find a uniform in a shader program
amos_uniform_t* amos_shader_program_get_uniform(
    amos_shader_program_t* program,
    const char* name
) {
    if (!program || !name) {
        return NULL;
    }
    
    for (int i = 0; i < program->uniform_count; i++) {
        if (strcmp(program->uniforms[i].name, name) == 0) {
            return &program->uniforms[i];
        }
    }
    
    return NULL;
}

// Find an attribute in a shader program
amos_attribute_t* amos_shader_program_get_attribute(
    amos_shader_program_t* program,
    const char* name
) {
    if (!program || !name) {
        return NULL;
    }
    
    for (int i = 0; i < program->attribute_count; i++) {
        if (strcmp(program->attributes[i].name, name) == 0) {
            return &program->attributes[i];
        }
    }
    
    return NULL;
}

// Set a float uniform value
bool amos_shader_program_set_uniform_float(
    amos_shader_program_t* program,
    const char* name,
    float value
) {
    amos_uniform_t* uniform = amos_shader_program_get_uniform(program, name);
    
    if (!uniform || uniform->type != AMOS_UNIFORM_FLOAT) {
        return false;
    }
    
    *((float*)uniform->data) = value;
    return true;
}

// Set a vec3 uniform value
bool amos_shader_program_set_uniform_vec3(
    amos_shader_program_t* program,
    const char* name,
    const amos_vec3_t* value
) {
    amos_uniform_t* uniform = amos_shader_program_get_uniform(program, name);
    
    if (!uniform || uniform->type != AMOS_UNIFORM_VEC3 || !value) {
        return false;
    }
    
    memcpy(uniform->data, value, sizeof(amos_vec3_t));
    return true;
}

// Set a vec4 uniform value
bool amos_shader_program_set_uniform_vec4(
    amos_shader_program_t* program,
    const char* name,
    const amos_vec4_t* value
) {
    amos_uniform_t* uniform = amos_shader_program_get_uniform(program, name);
    
    if (!uniform || uniform->type != AMOS_UNIFORM_VEC4 || !value) {
        return false;
    }
    
    memcpy(uniform->data, value, sizeof(amos_vec4_t));
    return true;
}

// Set a matrix uniform value
bool amos_shader_program_set_uniform_mat4(
    amos_shader_program_t* program,
    const char* name,
    const amos_mat4_t* value
) {
    amos_uniform_t* uniform = amos_shader_program_get_uniform(program, name);
    
    if (!uniform || uniform->type != AMOS_UNIFORM_MAT4 || !value) {
        return false;
    }
    
    memcpy(uniform->data, value, sizeof(amos_mat4_t));
    return true;
}

// Set an integer uniform value
bool amos_shader_program_set_uniform_int(
    amos_shader_program_t* program,
    const char* name,
    int value
) {
    amos_uniform_t* uniform = amos_shader_program_get_uniform(program, name);
    
    if (!uniform || uniform->type != AMOS_UNIFORM_INT) {
        return false;
    }
    
    *((int*)uniform->data) = value;
    return true;
}

// Process a vertex through the vertex shader
void amos_shader_process_vertex(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out
) {
    if (!program || !vertex_in || !position_out || !varying_out) {
        return;
    }
    
    program->vertex_shader(program, vertex_in, position_out, varying_out);
}

// Process a fragment through the fragment shader
void amos_shader_process_fragment(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out
) {
    if (!program || !varying_in || !color_out) {
        return;
    }
    
    program->fragment_shader(program, varying_in, color_out);
}

// Interpolate varying data between three vertices
void amos_shader_interpolate_varying(
    const void* v0,
    const void* v1,
    const void* v2,
    const amos_vec3_t* barycentric,
    int size,
    void* result
) {
    if (!v0 || !v1 || !v2 || !barycentric || !result || size <= 0) {
        return;
    }
    
    // Cast to byte pointers for byte-by-byte interpolation
    const unsigned char* b0 = (const unsigned char*)v0;
    const unsigned char* b1 = (const unsigned char*)v1;
    const unsigned char* b2 = (const unsigned char*)v2;
    unsigned char* dst = (unsigned char*)result;
    
    // Interpolate each byte
    for (int i = 0; i < size; i++) {
        float interpolated = 
            b0[i] * barycentric->x +
            b1[i] * barycentric->y +
            b2[i] * barycentric->z;
        
        dst[i] = (unsigned char)interpolated;
    }
}