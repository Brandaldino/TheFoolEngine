#pragma once

#include "TheFoolEngine/Core/Base.h"

#include <string>
#include <sstream>
#include <functional>



namespace TheFoolEngine {
	// 统筹记录所有的事件类型
	enum class EventType {
		None = 0,
		// 窗口事件
		WindowClose,WindowResize,WindowFocus,WindowLostFocus,WindowMoved,
		// 应用事件
		AppTick,AppUpdate,AppRender,
		// 按键事件
		KeyPressed,KeyReleased,KeyTyped,
		// 鼠标事件
		MouseKeyPressed,MouseKeyReleased,MouseMoved,MouseScrolled
	};

	// 所有的事件种类（归属）
	enum EventCategory {
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyBoard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};

#define EVENT_CLASS_TYPE(type)	static EventType GetStaticType(){return EventType::##type;}\
								virtual EventType GetEventType() const override {return GetStaticType();}\
								virtual const char* GetName() const override{return #type;}

#define EVENT_CLASS_CATEGORY(category)	virtual int GetCategoryFlags() const override{ return category;}

	class THEFOOLENGINE_API Event {
		friend class EventDispatcher;
	public:
		virtual ~Event() = default;

		bool Handled = false; // 事件是否已被处理

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) {
			return GetCategoryFlags() & category;
		}
	};

	// 事件分发器
	class EventDispatcher {
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) :m_Event(event) {}

		template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.GetEventType() == T::GetStaticType()) {
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}