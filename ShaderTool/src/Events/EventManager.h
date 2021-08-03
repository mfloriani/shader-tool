#pragma once

#include "Patterns/ISubject.h"

#include <queue>

class EventManager : ISubject
{
public:
	~EventManager() {}

	static EventManager& Get()
	{
		static EventManager instance;
		return instance;
	}

	virtual void Attach(IObserver* observer) override;
	virtual void Detach(IObserver* observer) override;

	void Enqueue(std::shared_ptr<Event> e);
	void NotifyQueuedEvents();
	void Enable(bool enabled) { _IsEnabled = enabled; }

private:
	EventManager() = default;
	
	virtual void Notify(Event* e) override;

private:
	bool _IsEnabled{ true };
	std::vector<IObserver*> _Observers;
	std::queue<std::shared_ptr<Event>> _EventQueue;
};

#define EVENT_MANAGER EventManager::Get()