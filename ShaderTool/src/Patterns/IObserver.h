#pragma once

#include "Events/Event.h"

class IObserver
{
public:
	virtual ~IObserver() {}
	virtual void OnEvent(Event* e) = 0;
};