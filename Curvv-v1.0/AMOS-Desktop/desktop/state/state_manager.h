/*
 * state_manager.h - AMOS Desktop State Manager Interface
 *
 * This header defines the public interface for the state management
 * component of the AMOS desktop environment. It provides functionality
 * for storing and retrieving application state between sessions.
 */

#ifndef AMOS_STATE_MANAGER_H
#define AMOS_STATE_MANAGER_H

#include <stdbool.h>

/* Available value types for state data */
typedef enum {
    STATE_TYPE_NONE,
    STATE_TYPE_INT,
    STATE_TYPE_FLOAT,
    STATE_TYPE_STRING,
    STATE_TYPE_BOOL,
    STATE_TYPE_BINARY
} state_value_type_t;

/* Value structure to hold state data */
typedef struct {
    state_value_type_t type;
    union {
        int int_value;
        float float_value;
        char* string_value;
        bool bool_value;
        struct {
            void* data;
            int size;
        } binary_value;
    } data;
} state_value_t;

/*
 * Initialize the state manager
 * Returns 0 on success, -1 on failure
 */
int state_manager_init(void);

/*
 * Clean up state manager resources
 */
void state_manager_cleanup(void);

/*
 * Create a new state session for an application
 * app_name: Name of the application (must be unique)
 * Returns session handle on success, NULL on failure
 */
void* state_create_session(const char* app_name);

/*
 * Destroy a state session
 */
void state_destroy_session(void* session);

/*
 * Save the current state of a session
 * Returns 0 on success, -1 on failure
 */
int state_save_session(void* session);

/*
 * Load the previously saved state of a session
 * Returns 0 on success, -1 on failure
 */
int state_load_session(void* session);

/*
 * Set an integer value in the session
 */
void state_set_int(void* session, const char* key, int value);

/*
 * Get an integer value from the session
 * default_value: Value to return if key not found
 */
int state_get_int(void* session, const char* key, int default_value);

/*
 * Set a float value in the session
 */
void state_set_float(void* session, const char* key, float value);

/*
 * Get a float value from the session
 * default_value: Value to return if key not found
 */
float state_get_float(void* session, const char* key, float default_value);

/*
 * Set a string value in the session
 */
void state_set_string(void* session, const char* key, const char* value);

/*
 * Get a string value from the session
 * default_value: Value to return if key not found
 * Returns allocated string that must be freed by caller
 */
char* state_get_string(void* session, const char* key, const char* default_value);

/*
 * Set a boolean value in the session
 */
void state_set_bool(void* session, const char* key, bool value);

/*
 * Get a boolean value from the session
 * default_value: Value to return if key not found
 */
bool state_get_bool(void* session, const char* key, bool default_value);

/*
 * Set binary data in the session
 */
void state_set_binary(void* session, const char* key, const void* data, int size);

/*
 * Get binary data from the session
 * data: Pointer to receive the allocated data (must be freed by caller)
 * size: Pointer to receive the size of the data
 * Returns 0 on success, -1 if key not found
 */
int state_get_binary(void* session, const char* key, void** data, int* size);

/*
 * Check if a key exists in the session
 */
bool state_has_key(void* session, const char* key);

/*
 * Remove a key from the session
 */
void state_remove_key(void* session, const char* key);

/*
 * Clear all keys from the session
 */
void state_clear(void* session);

/*
 * Get the type of a key's value
 */
state_value_type_t state_get_type(void* session, const char* key);

/*
 * Get an array of all keys in the session
 * count: Pointer to receive the number of keys
 * Returns array of strings that must be freed by caller
 */
char** state_get_keys(void* session, int* count);

/*
 * Free an array of keys returned by state_get_keys
 */
void state_free_keys(char** keys, int count);

#endif /* AMOS_STATE_MANAGER_H */