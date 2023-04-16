#include "GameObject.h"
#include "Command.h"

dae::Command::Command()
{
}

void dae::Command::internal_execute()
{
	OnExecute();
}
