/*
 * Event.h
 *
 *  Created on: Sep 5, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <list>

namespace nmea {

template<class>
class EventHandler;

template<class>
class Event;

template<typename... Args>
class EventHandler<void(Args...)> {
	friend Event<void(Args...)>;

private:
	// Static members
	static uint64_t LastID;

	// Properties
	typename Event<void(Args...)>::ListIterator iterator_;
	uint64_t                                    ID_;
	std::function<void(Args...)>                handler_;

public:
	// Typenames
	using FunctionPointer = void (*)(Args...);

	// Functions
	explicit EventHandler(std::function<void(Args...)> handler)
	    : iterator_()
	    , ID_(++LastID)
	    , handler_(handler)
	{
	}

	EventHandler(const EventHandler& ref)
	    : iterator_(ref.iterator_)
	    , ID_(ref.ID_)
	    , handler_(ref.handler_)
	{
	}

	EventHandler& operator=(EventHandler ref)
	{
		std::swap(iterator_, ref.iterator_);
		std::swap(handler_, ref.handler_);
		std::swap(ID_, ref.ID_);
		return *this;
	}

	void operator()(Args... args)
	{
		handler_(args...);
	}

	bool operator==(const EventHandler& ref)
	{
		return ID_ == ref.ID;
	}

	bool operator!=(const EventHandler& ref)
	{
		return ID_ != ref.ID;
	}

	uint64_t getID()
	{
		return ID_;
	}

	// Returns function pointer to the underlying function
	// or null if it's not a function but implements operator()
	FunctionPointer* getFunctionPointer()
	{
		FunctionPointer* ptr = handler_.template target<FunctionPointer>();
		return ptr;
	}
};

template<typename... Args>
uint64_t EventHandler<void(Args...)>::LastID = 0;

template<typename... Args>
class Event<void(Args...)> {
	friend EventHandler<void(Args...)>;

private:
	// Typenames
	using ListIterator = typename std::list<EventHandler<void(Args...)>>::iterator;

	// Properties
	std::list<EventHandler<void(Args...)>> handlers_;

	// Functions
	void _copy(const Event& ref)
	{
		if ( &ref != this ) {
			handlers_ = ref.handlers;
		}
	};

	bool removeHandler(ListIterator handlerIter)
	{
		if ( handlerIter == handlers_.end() ) {
			return false;
		}

		handlers_.erase(handlerIter);
		return true;
	};

public:
	// Properties
	bool enabled{ true };

	// Functions

	void call(Args... args)
	{
		if ( enabled ) {
			for ( auto& handler: handlers_ ) {
				handler(args...);
			}
		}
	}

	EventHandler<void(Args...)> registerHandler(EventHandler<void(Args...)> eventHandler)
	{
		for ( const auto& handler: handlers_ ) {
			if ( handler == eventHandler ) {
				eventHandler.iterator_ = handlers_.insert(handlers_.end(), eventHandler);
				break;
			}
		}
		return eventHandler;
	}

	EventHandler<void(Args...)> registerHandler(std::function<void(Args...)> handler)
	{
		EventHandler<void(Args...)> wrapper(handler);
		wrapper.iterator_ = handlers_.insert(handlers_.end(), wrapper);
		return wrapper;
	}

	bool removeHandler(EventHandler<void(Args...)>& handler)
	{
		bool sts          = removeHandler(handler.iterator_);
		handler.iterator_ = handlers_.end();
		return sts;
	};

	void clear()
	{
		for ( const auto& handler: handlers_ ) {
			handler.iterator_ = handlers_.end();
		}
		handlers_.clear();
	};

	void                        operator()(Args... args) { return call(args...); };
	EventHandler<void(Args...)> operator+=(EventHandler<void(Args...)> handler) { return registerHandler(handler); };
	EventHandler<void(Args...)> operator+=(std::function<void(Args...)> handler) { return registerHandler(handler); };
	bool                        operator-=(EventHandler<void(Args...)>& handler) { return removeHandler(handler); };
	bool                        operator-=(uint64_t handlerID) { return removeHandler(handlerID); };
};

} // namespace nmea
