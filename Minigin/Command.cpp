#include "GameObject.h"
#include "Command.h"

dae::Command::Command(GameObject* owner)
{
	Owner = owner;
}

void dae::Command::internal_execute()
{
	OnExecute();
}
