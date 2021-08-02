#include "pch.h"
#include "EventManager.h"

void EventManager::Attach(IObserver* observer)
{
	auto it = std::find(_Observers.begin(), _Observers.end(), observer);
	if (it != _Observers.end())
	{
		LOG_WARN("EventManager::Attach -> Observer is already attached! Skipping...");
		return;
	}
	LOG_TRACE("EventManager::Attach -> Observer attached");
	_Observers.push_back(observer);
}

void EventManager::Detach(IObserver* observer)
{
	auto it = std::find(_Observers.begin(), _Observers.end(), observer);
	if (it == _Observers.end())
	{
		LOG_WARN("EventManager::Detach -> Observer is not attached! Skipping...");
		return;
	}
	LOG_TRACE("EventManager::Detach -> Observer detached");
	_Observers.erase(it); // TODO: swap with last element to erase it?
}

void EventManager::Notify(Event* e)
{
	for (auto observer : _Observers)
	{
		observer->OnEvent(e);
	}
}
