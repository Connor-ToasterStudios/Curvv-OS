/**
 * AMOS Desktop OS - 3D Math Operations
 * 
 * This file implements basic 3D math operations for the AMOS 3D renderer.
 */

#include "renderer3d.h"
#include <math.h>

// Vector operations
void amos_vec3_add(const amos_vec3_t* a, const amos_vec3_t* b, amos_vec3_t* result) {
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
}

void amos_vec3_subtract(const amos_vec3_t* a, const amos_vec3_t* b, amos_vec3_t* result) {
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

void amos_vec3_multiply(const amos_vec3_t* a, float scalar, amos_vec3_t* result) {
    result->x = a->x * scalar;
    result->y = a->y * scalar;
    result->z = a->z * scalar;
}

float amos_vec3_length(const amos_vec3_t* v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

void amos_vec3_normalize(const amos_vec3_t* v, amos_vec3_t* result) {
    float length = amos_vec3_length(v);
    if (length > 0.0001f) {
        float inv_length = 1.0f / length;
        result->x = v->x * inv_length;
        result->y = v->y * inv_length;
        result->z = v->z * inv_length;
    } else {
        // Avoid division by zero
        result->x = 0.0f;
        result->y = 0.0f;
        result->z = 0.0f;
    }
}

float amos_vec3_dot(const amos_vec3_t* a, const amos_vec3_t* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

void amos_vec3_cross(const amos_vec3_t* a, const amos_vec3_t* b, amos_vec3_t* result) {
    result->x = a->y * b->z - a->z * b->y;
    result->y = a->z * b->x - a->x * b->z;
    result->z = a->x * b->y - a->y * b->x;
}

// Matrix operations
void amos_mat4_identity(amos_mat4_t* m) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m->m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

void amos_mat4_multiply(const amos_mat4_t* a, const amos_mat4_t* b, amos_mat4_t* result) {
    amos_mat4_t temp;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp.m[i][j] = 
                a->m[i][0] * b->m[0][j] +
                a->m[i][1] * b->m[1][j] +
                a->m[i][2] * b->m[2][j] +
                a->m[i][3] * b->m[3][j];
        }
    }
    
    *result = temp;
}

void amos_mat4_translate(amos_mat4_t* m, float x, float y, float z) {
    amos_mat4_t translation;
    amos_mat4_identity(&translation);
    
    translation.m[0][3] = x;
    translation.m[1][3] = y;
    translation.m[2][3] = z;
    
    amos_mat4_t temp = *m;
    amos_mat4_multiply(&temp, &translation, m);
}

void amos_mat4_rotate(amos_mat4_t* m, float angle, float x, float y, float z) {
    float c = cosf(angle);
    float s = sinf(angle);
    float one_minus_c = 1.0f - c;
    
    // Normalize axis
    float length = sqrtf(x * x + y * y + z * z);
    if (length < 0.0001f) {
        return;  // Invalid rotation axis
    }
    
    x /= length;
    y /= length;
    z /= length;
    
    amos_mat4_t rotation;
    
    rotation.m[0][0] = x * x * one_minus_c + c;
    rotation.m[0][1] = x * y * one_minus_c - z * s;
    rotation.m[0][2] = x * z * one_minus_c + y * s;
    rotation.m[0][3] = 0.0f;
    
    rotation.m[1][0] = y * x * one_minus_c + z * s;
    rotation.m[1][1] = y * y * one_minus_c + c;
    rotation.m[1][2] = y * z * one_minus_c - x * s;
    rotation.m[1][3] = 0.0f;
    
    rotation.m[2][0] = z * x * one_minus_c - y * s;
    rotation.m[2][1] = z * y * one_minus_c + x * s;
    rotation.m[2][2] = z * z * one_minus_c + c;
    rotation.m[2][3] = 0.0f;
    
    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;
    
    amos_mat4_t temp = *m;
    amos_mat4_multiply(&temp, &rotation, m);
}

void amos_mat4_scale(amos_mat4_t* m, float x, float y, float z) {
    amos_mat4_t scale;
    amos_mat4_identity(&scale);
    
    scale.m[0][0] = x;
    scale.m[1][1] = y;
    scale.m[2][2] = z;
    
    amos_mat4_t temp = *m;
    amos_mat4_multiply(&temp, &scale, m);
}

void amos_mat4_perspective(amos_mat4_t* m, float fov, float aspect, float near_clip, float far_clip) {
    float f = 1.0f / tanf(fov * 0.5f);
    float range_inv = 1.0f / (near_clip - far_clip);
    
    amos_mat4_identity(m);
    
    m->m[0][0] = f / aspect;
    m->m[1][1] = f;
    m->m[2][2] = (near_clip + far_clip) * range_inv;
    m->m[2][3] = 2.0f * near_clip * far_clip * range_inv;
    m->m[3][2] = -1.0f;
    m->m[3][3] = 0.0f;
}

void amos_mat4_look_at(amos_mat4_t* m, const amos_vec3_t* eye, const amos_vec3_t* center, const amos_vec3_t* up) {
    amos_vec3_t f, s, u;
    
    // Calculate forward vector
    amos_vec3_subtract(center, eye, &f);
    amos_vec3_normalize(&f, &f);
    
    // Calculate right vector
    amos_vec3_cross(&f, up, &s);
    amos_vec3_normalize(&s, &s);
    
    // Calculate up vector
    amos_vec3_cross(&s, &f, &u);
    
    amos_mat4_identity(m);
    
    m->m[0][0] = s.x;
    m->m[0][1] = s.y;
    m->m[0][2] = s.z;
    
    m->m[1][0] = u.x;
    m->m[1][1] = u.y;
    m->m[1][2] = u.z;
    
    m->m[2][0] = -f.x;
    m->m[2][1] = -f.y;
    m->m[2][2] = -f.z;
    
    m->m[0][3] = -amos_vec3_dot(&s, eye);
    m->m[1][3] = -amos_vec3_dot(&u, eye);
    m->m[2][3] = amos_vec3_dot(&f, eye);
}

void amos_mat4_transform_vec3(const amos_mat4_t* m, const amos_vec3_t* v, amos_vec3_t* result) {
    float w = 
        v->x * m->m[0][3] +
        v->y * m->m[1][3] +
        v->z * m->m[2][3] +
        m->m[3][3];
    
    float x = 
        v->x * m->m[0][0] +
        v->y * m->m[1][0] +
        v->z * m->m[2][0] +
        m->m[3][0];
    
    float y = 
        v->x * m->m[0][1] +
        v->y * m->m[1][1] +
        v->z * m->m[2][1] +
        m->m[3][1];
    
    float z = 
        v->x * m->m[0][2] +
        v->y * m->m[1][2] +
        v->z * m->m[2][2] +
        m->m[3][2];
    
    // Perspective divide
    if (fabsf(w) > 0.00001f) {
        float inv_w = 1.0f / w;
        result->x = x * inv_w;
        result->y = y * inv_w;
        result->z = z * inv_w;
    } else {
        result->x = 0.0f;
        result->y = 0.0f;
        result->z = 0.0f;
    }
}

void amos_mat4_transform_vec4(const amos_mat4_t* m, const amos_vec4_t* v, amos_vec4_t* result) {
    result->x = 
        v->x * m->m[0][0] +
        v->y * m->m[1][0] +
        v->z * m->m[2][0] +
        v->w * m->m[3][0];
    
    result->y = 
        v->x * m->m[0][1] +
        v->y * m->m[1][1] +
        v->z * m->m[2][1] +
        v->w * m->m[3][1];
    
    result->z = 
        v->x * m->m[0][2] +
        v->y * m->m[1][2] +
        v->z * m->m[2][2] +
        v->w * m->m[3][2];
    
    result->w = 
        v->x * m->m[0][3] +
        v->y * m->m[1][3] +
        v->z * m->m[2][3] +
        v->w * m->m[3][3];
}