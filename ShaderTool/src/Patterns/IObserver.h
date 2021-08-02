#pragma once

struct Event;

class IObserver
{
public:
	virtual ~IObserver() {}
	virtual void OnEvent(Event* e) = 0;
};