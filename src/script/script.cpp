#include "wander/script/script.h"
#include "wander/core/log.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace wander {

// Trampoline: Lua calls this, we look up the real C++ function
static int lua_trampoline(lua_State* L) {
    auto* func = static_cast<ScriptFunction*>(lua_touserdata(L, lua_upvalueindex(1)));
    return (*func)(L);
}

bool ScriptEngine::init() {
    L_ = luaL_newstate();
    if (!L_) {
        LOG_ERROR("Script: failed to create Lua state");
        return false;
    }

    luaL_openlibs(L_);
    LOG_INFO("Script engine initialized (Lua %s.%s)",
        LUA_VERSION_MAJOR, LUA_VERSION_MINOR);
    return true;
}

void ScriptEngine::shutdown() {
    if (L_) {
        lua_close(L_);
        L_ = nullptr;
    }
    bindings_.clear();
    loaded_files_.clear();
}

bool ScriptEngine::load_file(const char* path) {
    if (!L_) return false;

    if (luaL_dofile(L_, path) != LUA_OK) {
        LOG_ERROR("Script: %s", lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }

    // Track for hot-reload
    loaded_files_.push_back(path);
    LOG_DEBUG("Script: loaded %s", path);
    return true;
}

bool ScriptEngine::exec(const char* code) {
    if (!L_) return false;

    if (luaL_dostring(L_, code) != LUA_OK) {
        LOG_ERROR("Script: %s", lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

void ScriptEngine::bind(const char* name, ScriptFunction func) {
    if (!L_) return;

    // Store the function in our map (keeps it alive)
    bindings_[name] = std::move(func);

    // Push a light userdata pointing to our stored function, then push a closure
    lua_pushlightuserdata(L_, &bindings_[name]);
    lua_pushcclosure(L_, lua_trampoline, 1);
    lua_setglobal(L_, name);
}

bool ScriptEngine::call(const char* func_name) {
    if (!L_) return false;

    lua_getglobal(L_, func_name);
    if (!lua_isfunction(L_, -1)) {
        lua_pop(L_, 1);
        return false;
    }

    if (lua_pcall(L_, 0, 0, 0) != LUA_OK) {
        LOG_ERROR("Script call '%s': %s", func_name, lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

bool ScriptEngine::call(const char* func_name, f32 arg) {
    if (!L_) return false;

    lua_getglobal(L_, func_name);
    if (!lua_isfunction(L_, -1)) {
        lua_pop(L_, 1);
        return false;
    }

    lua_pushnumber(L_, arg);
    if (lua_pcall(L_, 1, 0, 0) != LUA_OK) {
        LOG_ERROR("Script call '%s': %s", func_name, lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

void ScriptEngine::set_global(const char* name, f32 value) {
    if (!L_) return;
    lua_pushnumber(L_, value);
    lua_setglobal(L_, name);
}

void ScriptEngine::set_global(const char* name, i32 value) {
    if (!L_) return;
    lua_pushinteger(L_, value);
    lua_setglobal(L_, name);
}

void ScriptEngine::set_global(const char* name, const char* value) {
    if (!L_) return;
    lua_pushstring(L_, value);
    lua_setglobal(L_, name);
}

f32 ScriptEngine::get_global_float(const char* name) {
    if (!L_) return 0.0f;
    lua_getglobal(L_, name);
    f32 v = static_cast<f32>(lua_tonumber(L_, -1));
    lua_pop(L_, 1);
    return v;
}

i32 ScriptEngine::get_global_int(const char* name) {
    if (!L_) return 0;
    lua_getglobal(L_, name);
    i32 v = static_cast<i32>(lua_tointeger(L_, -1));
    lua_pop(L_, 1);
    return v;
}

std::string ScriptEngine::get_global_string(const char* name) {
    if (!L_) return "";
    lua_getglobal(L_, name);
    const char* s = lua_tostring(L_, -1);
    std::string result = s ? s : "";
    lua_pop(L_, 1);
    return result;
}

void ScriptEngine::reload_all() {
    if (!L_) return;
    LOG_INFO("Script: hot-reloading %zu files", loaded_files_.size());

    // Re-execute all loaded files
    auto files = loaded_files_;  // Copy since load_file appends
    loaded_files_.clear();
    for (auto& path : files) {
        load_file(path.c_str());
    }
}

// Helper functions
void script_push(lua_State* L, f32 value) { lua_pushnumber(L, value); }
void script_push(lua_State* L, i32 value) { lua_pushinteger(L, value); }
void script_push(lua_State* L, const char* value) { lua_pushstring(L, value); }
f32  script_to_float(lua_State* L, i32 index) { return static_cast<f32>(lua_tonumber(L, index)); }
i32  script_to_int(lua_State* L, i32 index) { return static_cast<i32>(lua_tointeger(L, index)); }
const char* script_to_string(lua_State* L, i32 index) { return lua_tostring(L, index); }

} // namespace wander
