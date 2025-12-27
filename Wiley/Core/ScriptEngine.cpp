#include "ScriptEngine.h"
#include <Windows.h>


ScriptState::ScriptState()
{
	luaState.open_libraries(sol::lib::base, sol::lib::math,sol::lib::table);
	luaState.set_function("get_frame_width", [&]() {});

}

ScriptState::~ScriptState()
{
	
}

void ScriptState::SetFunction(const std::string& functionName, std::function<void()> function)
{
	luaState.set_function(functionName, function);
}

void ScriptState::RunScript(const std::string& command)
{
	luaState.script(command);
}

void ScriptState::LoadScriptFile(const std::string& name)
{
	luaState.script_file(name);
}
