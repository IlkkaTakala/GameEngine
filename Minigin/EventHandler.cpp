#include "EventHandler.h"
#include <queue>

namespace dae {
	static std::queue<Event> EventQueue;

	static std::list<EventListener*> Listeners;
}
dae::EventListener::EventListener(const std::vector<EventType>& eventsToListen)
{
	EventTypes = eventsToListen;
	Listeners.push_back(this);
}

dae::EventListener::~EventListener()
{
	Listeners.remove(this);
}

void dae::EventListener::RemoveEventType(EventType event)
{
	std::erase(EventTypes, event);
}

void dae::EventListener::AddEventType(EventType event)
{
	EventTypes.push_back(event);
}

void dae::EventListener::Notify(Event event)
{
	if (std::find(EventTypes.begin(), EventTypes.end(), event.type) != EventTypes.end()) {
		Notified(event.type, event.object);
	}
}

void dae::EventHandler::FireEvent(EventType event, GameObject* object)
{
	EventQueue.push({ event, object });
}

void dae::EventHandler::ProcessEvents()
{
	while (!EventQueue.empty()) {
		Event e = EventQueue.front();
		EventQueue.pop();

		for (auto& l : Listeners) {
			l->Notify(e);
		}
	}
}
