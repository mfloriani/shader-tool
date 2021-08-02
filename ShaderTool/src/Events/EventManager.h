#pragma once

#include "Patterns/ISubject.h"

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
	virtual void Notify(Event* e) override;

private:
	EventManager() = default;

private:
	std::vector<IObserver*> _Observers;
};

#define EVENT_MANAGER EventManager::Get()