#pragma once

struct Event
{
	virtual ~Event() {}
};

struct LinkCreatedEvent : public Event
{
	LinkCreatedEvent(int from, int to) : from(from), to(to) {}
	int from, to;
};

struct LinkDeletedEvent : public Event
{
	LinkDeletedEvent(int from, int to) : from(from), to(to) {}
	int from, to;
};

struct ShaderUpdatedEvent : public Event
{
	ShaderUpdatedEvent(int newShaderIndex) : index(newShaderIndex) {}
	int index;
};