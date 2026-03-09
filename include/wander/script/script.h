#pragma once

#include "wander/core/types.h"
#include <string>
#include <functional>
#include <unordered_map>

struct lua_State;

namespace wander {

// C++ function callable from Lua
using ScriptFunction = std::function<int(lua_State*)>;

// Script system — manages Lua VM and bindings
class ScriptEngine {
public:
    bool init();
    void shutdown();

    // Load and execute a Lua script file
    bool load_file(const char* path);

    // Execute a Lua string
    bool exec(const char* code);

    // Register a C++ function callable from Lua
    void bind(const char* name, ScriptFunction func);

    // Call a Lua function by name
    bool call(const char* func_name);
    bool call(const char* func_name, f32 arg);

    // Get/set global variables
    void set_global(const char* name, f32 value);
    void set_global(const char* name, i32 value);
    void set_global(const char* name, const char* value);
    f32  get_global_float(const char* name);
    i32  get_global_int(const char* name);
    std::string get_global_string(const char* name);

    // Hot-reload: reload all loaded scripts
    void reload_all();

    // Get the raw Lua state (for advanced use)
    lua_State* state() { return L_; }

private:
    lua_State* L_ = nullptr;
    std::unordered_map<std::string, ScriptFunction> bindings_;
    std::vector<std::string> loaded_files_;
};

// Convenience: push/pop values to Lua stack
void script_push(lua_State* L, f32 value);
void script_push(lua_State* L, i32 value);
void script_push(lua_State* L, const char* value);
f32  script_to_float(lua_State* L, i32 index);
i32  script_to_int(lua_State* L, i32 index);
const char* script_to_string(lua_State* L, i32 index);

} // namespace wander
