#pragma once

struct Event
{
	virtual ~Event() {}
};

struct LinkCreatedEvent : public Event
{
	int from, to;
};

struct LinkDeletedEvent : public Event
{
	int from, to;
};