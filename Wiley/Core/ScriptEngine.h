#pragma once
#include "sol/sol.hpp"

#include <functional>
#include <string>
#include <memory>
#include <future>

class ScriptState
{
	public:
		ScriptState();
		~ScriptState();

		void SetFunction(const std::string& functionName, std::function<void()> function);

		template<typename F>
		void SetFunction(const std::string& functionName, F&& function)
		{
			luaState.set_function(functionName, std::forward<F>(function));
		}
		
		template<typename ...Args>
		void DefineEnum(const std::string& enumName, Args&& ...args)
		{
			luaState.new_enum(
				enumName, std::forward<Args>(args)...
			);
		}

		template<typename Class, typename ...Args>
		void DefineUserType(Args&& ...args)
		{
			luaState.new_usertype<Class>(std::forward<Args>(args)...);
		}

		template<typename ...Args>
		void SetConstant(Args&& ...args) {
			luaState.set(std::forward<Args>(args)...);
		}

		void RunScript(const std::string& command);
		void LoadScriptFile(const std::string& name);

		sol::state& GetInternalState() { return luaState; }

private:
	sol::state luaState;
};


