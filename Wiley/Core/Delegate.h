#pragma once
//Created on 12/21/2025 6:50~

#include "defines.h"

#include <functional>
#include <concepts>
#include <utility>

namespace Wiley {

#define DECLARE_DELEGATE(name,...)\
	using name = Delegate<void(__VA_ARGS__)>;

#define DECLARE_DELEGATE_RETVALUE(name, _return, ...)\
	using name = Delegate<_return(__VA_ARGS__)>;

#define DECLARE_MULTICAST_DELEGATE(name, ...)\
	using name = MultiCastDelegate<__VA_ARGS__>;

#define DECLARE_EVENT(name,owner,...)\
	class name : public MultiCastDelegate<__VA_ARGS__>\
	{\
		private:\
		friend class owner;\
		using MultiCastDelegate<__VA_ARGS__>::Broadcast;\
		using MultiCastDelegate<__VA_ARGS__>::RemoveAll;\
		using MultiCastDelegate<__VA_ARGS__>::Remove;\
	};


	template<typename...>
	class Delegate;

	template<typename DelegateType, typename _Func>
	concept IsConstructibleToDelegateType = std::is_constructible_v<DelegateType, _Func>;

	template<typename _Return, typename... Args>
	class Delegate<_Return(Args...)>
	{
	public:
		using DelegateType = std::function<_Return(Args...)>;
		Delegate() = default;
		Delegate(const Delegate&) = default;
		~Delegate() = default;

		template<typename _Func>
			requires IsConstructibleToDelegateType<DelegateType, _Func>
		void BindLambda(_Func&& funcPtr) {
			callback = funcPtr;
		}

		void BindStatic(_Return(*funcPtr)(Args...)) {
			callback = funcPtr;
		}

		template<typename T>
		void BindMemberFunction(_Return(T::* memFunc)(Args...), T& instance) {
			callback = [&instance, memFunc](Args&&... args) mutable -> _Return {return (instance.*memFunc)(std::forward<Args>(args)...); };
		}

		void Unbind() {
			callback = nullptr;
		}

		_Return Execute(Args... args) const {
			return callback(args...);
		}

		_Return ExecuteIfBound(Args... args) const {
			return IsBound() ? callback(args...) : _Return();
		}

		bool IsBound() const {
			return (callback != nullptr);
		}

		template<typename _Func>
			requires IsConstructibleToDelegateType<DelegateType, _Func>
		static Delegate CreateLambda(_Func&& funcPtr) {
			Delegate delegate;
			delegate.BindLambda(std::forward<_Func>(funcPtr));
			return delegate;
		}

		template<typename _Func>
			requires IsConstructibleToDelegateType<DelegateType, _Func>
		static Delegate CreateStatic(_Return(*funcPtr)(Args...)) {
			Delegate delegate;
			delegate.BindStatic(funcPtr);
			return delegate;
		}

		template<typename T>
		static Delegate CreateMemberFunction(_Return(T::* memFunc)(Args...), T& instance) {
			Delegate delegate;
			delegate.BindMemberFunction(memFunc, instance);
			return delegate;
		}

		_Return operator()(Args... args)const {
			return callback(args...);
		}

	private:
		DelegateType callback;
	};


	class DelegateHandle {

	public:
		DelegateHandle() :id(InvalidID) {}
		explicit DelegateHandle(int) :id(GenerateID()) {};
		~DelegateHandle() = default;

		bool IsValid()const {
			return id != InvalidID;
		}
		void Reset() {
			id = InvalidID;
		}

		operator bool() {
			return id != InvalidID;
		}

		bool operator==(const DelegateHandle& other)const {
			return id == other.id;
		}
	private:
		inline static constexpr uint64_t InvalidID = uint64_t(-1);
		static uint64_t GenerateID() {
			static uint64_t idCounter = 0;
			return idCounter++;
		}
	private:
		uint64_t id;
	};


	template<typename ...Args>
	class MultiCastDelegate {
		using DelegateType = Delegate<void(Args...)>;
		using HandleDelegatePar = std::pair<DelegateHandle, DelegateType>;
	public:
		MultiCastDelegate() = default;
		~MultiCastDelegate() = default;

		WILEY_MAYBE_UNUSED DelegateHandle Add(DelegateType&& handle) {
			delegatePool.emplace_back(DelegateHandle(0), std::forward<DelegateType>(handle));
			return delegatePool.back().first;
		}

		WILEY_MAYBE_UNUSED DelegateHandle Add(const DelegateType& handler) {
			delegatePool.emplace_back(DelegateHandle(0), handler);
			return delegatePool.back().first;
		}

		template<typename _Func>
		WILEY_MAYBE_UNUSED DelegateHandle AddLambda(_Func&& funcPtr) {
			return Add(DelegateType::CreateLambda(funcPtr));
		}


		template<typename _Return>
		WILEY_MAYBE_UNUSED DelegateHandle AddStatic(_Return(*funcPtr)(Args...)) {
			return Add(DelegateType::CreateStatic(funcPtr));
		}

		template<typename T>
		WILEY_MAYBE_UNUSED DelegateHandle AddMember(void(T::* memFunc)(Args...), T& instance) {
			return Add(DelegateType::CreateMemberFunction(memFunc, instance));
		}

		WILEY_MAYBE_UNUSED bool Remove(DelegateHandle& handle)
		{
			if (handle.IsValid())
			{
				for (uint64_t i = 0; i < delegatePool.size(); ++i)
				{
					if (delegatePool[i].first == handle)
					{
						std::swap(delegatePool[i], delegatePool.back());
						delegatePool.pop_back();
						handle.Reset();
						return true;
					}
				}
			}
			return false;
		}

		void RemoveAll()
		{
			delegatePool.clear();
		}

		void Broadcast(Args... args)
		{
			for (uint64_t i = 0; i < delegatePool.size(); ++i)
			{
				if (delegatePool[i].first.IsValid()) delegatePool[i].second.ExecuteIfBound(std::forward<Args>(args)...);
			}
		}

		bool IsHandleBound(DelegateHandle const& handle) const
		{
			if (handle.IsValid())
			{
				for (uint64_t i = 0; i < delegatePool.size(); ++i)
				{
					if (delegatePool[i].first == handle) return true;
				}
			}
			return false;
		}


	private:
		std::vector<HandleDelegatePar> delegatePool;
	};

}