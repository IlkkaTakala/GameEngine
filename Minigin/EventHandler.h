#pragma once
#include <vector>
#include "Minigin.h"

namespace dae
{
typedef int EventType;
class GameObject;

struct Event
{
	EventType type{ 0 };
	GameObject* object{ nullptr };
};

class EventHandler
{
public:
	static void FireEvent(EventType event, GameObject* object);

private:
	friend class Minigin;
	static void ProcessEvents();
};

class EventListener
{
public:
	EventListener(const std::vector<EventType>& eventsToListen);
	~EventListener();

protected:
	friend class EventHandler;
	virtual void Notified(EventType event, GameObject* object) = 0;
	void RemoveEventType(EventType event);
	void AddEventType(EventType event);

private:
	std::vector<EventType> EventTypes;
	void Notify(Event event);
};


}