/**
 * AMOS Desktop OS - 3D Renderer Shader System
 * 
 * This file defines the shader system for the AMOS 3D renderer,
 * providing a programmable vertex and fragment processing pipeline.
 */

#ifndef AMOS_SHADERS_H
#define AMOS_SHADERS_H

#include "renderer3d.h"

// Forward declarations
typedef struct amos_shader_program_t amos_shader_program_t;
typedef struct amos_uniform_t amos_uniform_t;
typedef struct amos_attribute_t amos_attribute_t;

// Maximum number of uniforms per shader
#define AMOS_MAX_UNIFORMS 16

// Maximum number of attributes per shader
#define AMOS_MAX_ATTRIBUTES 8

// Maximum shader name length
#define AMOS_MAX_SHADER_NAME_LENGTH 64

// Uniform types
typedef enum {
    AMOS_UNIFORM_FLOAT,
    AMOS_UNIFORM_VEC2,
    AMOS_UNIFORM_VEC3,
    AMOS_UNIFORM_VEC4,
    AMOS_UNIFORM_MAT4,
    AMOS_UNIFORM_INT,
    AMOS_UNIFORM_SAMPLER2D
} amos_uniform_type_t;

// Attribute types
typedef enum {
    AMOS_ATTRIBUTE_FLOAT,
    AMOS_ATTRIBUTE_VEC2,
    AMOS_ATTRIBUTE_VEC3,
    AMOS_ATTRIBUTE_VEC4
} amos_attribute_type_t;

// Vertex shader function pointer
typedef void (*amos_vertex_shader_fn)(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out
);

// Fragment shader function pointer
typedef void (*amos_fragment_shader_fn)(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out
);

// Uniform structure
struct amos_uniform_t {
    char name[AMOS_MAX_SHADER_NAME_LENGTH];
    amos_uniform_type_t type;
    void* data;
    int size;  // Size in bytes
};

// Attribute structure
struct amos_attribute_t {
    char name[AMOS_MAX_SHADER_NAME_LENGTH];
    amos_attribute_type_t type;
    int offset;  // Offset in vertex structure
    int size;    // Size in bytes
};

// Shader program structure
struct amos_shader_program_t {
    char name[AMOS_MAX_SHADER_NAME_LENGTH];
    amos_vertex_shader_fn vertex_shader;
    amos_fragment_shader_fn fragment_shader;
    
    // Uniforms
    amos_uniform_t uniforms[AMOS_MAX_UNIFORMS];
    int uniform_count;
    
    // Attributes
    amos_attribute_t attributes[AMOS_MAX_ATTRIBUTES];
    int attribute_count;
    
    // Varying data size (for passing data between vertex and fragment shaders)
    int varying_size;
};

/**
 * Initialize a shader program
 * 
 * @param program Pointer to shader program structure
 * @param name Program name
 * @param vertex_shader Vertex shader function
 * @param fragment_shader Fragment shader function
 * @param varying_size Size of varying data in bytes
 * @return true if initialization was successful, false otherwise
 */
bool amos_shader_program_init(
    amos_shader_program_t* program,
    const char* name,
    amos_vertex_shader_fn vertex_shader,
    amos_fragment_shader_fn fragment_shader,
    int varying_size
);

/**
 * Add a uniform to a shader program
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @param type Uniform type
 * @param data Pointer to uniform data
 * @param size Size of uniform data in bytes
 * @return true if successful, false otherwise
 */
bool amos_shader_program_add_uniform(
    amos_shader_program_t* program,
    const char* name,
    amos_uniform_type_t type,
    void* data,
    int size
);

/**
 * Add an attribute to a shader program
 * 
 * @param program Pointer to shader program
 * @param name Attribute name
 * @param type Attribute type
 * @param offset Offset in vertex structure
 * @param size Size of attribute in bytes
 * @return true if successful, false otherwise
 */
bool amos_shader_program_add_attribute(
    amos_shader_program_t* program,
    const char* name,
    amos_attribute_type_t type,
    int offset,
    int size
);

/**
 * Find a uniform in a shader program
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @return Pointer to uniform, or NULL if not found
 */
amos_uniform_t* amos_shader_program_get_uniform(
    amos_shader_program_t* program,
    const char* name
);

/**
 * Find an attribute in a shader program
 * 
 * @param program Pointer to shader program
 * @param name Attribute name
 * @return Pointer to attribute, or NULL if not found
 */
amos_attribute_t* amos_shader_program_get_attribute(
    amos_shader_program_t* program,
    const char* name
);

/**
 * Set a float uniform value
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @param value Float value
 * @return true if successful, false otherwise
 */
bool amos_shader_program_set_uniform_float(
    amos_shader_program_t* program,
    const char* name,
    float value
);

/**
 * Set a vec3 uniform value
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @param value Pointer to vec3 value
 * @return true if successful, false otherwise
 */
bool amos_shader_program_set_uniform_vec3(
    amos_shader_program_t* program,
    const char* name,
    const amos_vec3_t* value
);

/**
 * Set a vec4 uniform value
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @param value Pointer to vec4 value
 * @return true if successful, false otherwise
 */
bool amos_shader_program_set_uniform_vec4(
    amos_shader_program_t* program,
    const char* name,
    const amos_vec4_t* value
);

/**
 * Set a matrix uniform value
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @param value Pointer to matrix value
 * @return true if successful, false otherwise
 */
bool amos_shader_program_set_uniform_mat4(
    amos_shader_program_t* program,
    const char* name,
    const amos_mat4_t* value
);

/**
 * Set an integer uniform value
 * 
 * @param program Pointer to shader program
 * @param name Uniform name
 * @param value Integer value
 * @return true if successful, false otherwise
 */
bool amos_shader_program_set_uniform_int(
    amos_shader_program_t* program,
    const char* name,
    int value
);

/**
 * Process a vertex through the vertex shader
 * 
 * @param program Pointer to shader program
 * @param vertex_in Input vertex
 * @param position_out Output position
 * @param varying_out Output varying data
 */
void amos_shader_process_vertex(
    const amos_shader_program_t* program,
    const amos_vertex_t* vertex_in,
    amos_vec4_t* position_out,
    void* varying_out
);

/**
 * Process a fragment through the fragment shader
 * 
 * @param program Pointer to shader program
 * @param varying_in Input varying data
 * @param color_out Output color
 */
void amos_shader_process_fragment(
    const amos_shader_program_t* program,
    const void* varying_in,
    amos_vec4_t* color_out
);

/**
 * Interpolate varying data between three vertices
 * 
 * @param v0 Varying data from first vertex
 * @param v1 Varying data from second vertex
 * @param v2 Varying data from third vertex
 * @param barycentric Barycentric coordinates
 * @param size Size of varying data in bytes
 * @param result Pointer to store interpolated result
 */
void amos_shader_interpolate_varying(
    const void* v0,
    const void* v1,
    const void* v2,
    const amos_vec3_t* barycentric,
    int size,
    void* result
);

#endif /* AMOS_SHADERS_H */