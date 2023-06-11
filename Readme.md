# Programming 4 - Game Engine
[Link to github: https://github.com/IlkkaTakala/GameEngine](https://github.com/IlkkaTakala/GameEngine)

The engine is built on Component based architecture, where gameobjects are populated using factory functions and component composition. All components inherit from BaseComponent-class through a intermediary Component template using CRTP. This allows for runtime polymorphism, although the use of dynamic_cast is discouraged as all component classes have a unique static identifier. All classes also have automatic default object that can be used for comparisons.  
  
## Components
The engine features an automatic object pool for all components. Components are created using a factory function, and should never be created with other methods. The object pool guarantees continuous memory allocation, but disables persistent pointers to objects. All components have some overridable virtual functions, but they may define their own Render function. As long as the function signature matches well enough, it will be automatically called during the render phase. Only these classes will be rendered.  
  
The same happens with ComponentUpdate function. For the moment it is overridable, but may later be changed to match the previous workflow. The ComponentUpdate will be called at the end of update sequentially for all objects that have overriden the function. This saves many loops, as not all components need the sequential update. 
  
## Component handles
To get a permanent handle to components, the engine contains a ComponentRef template which acts as a pointer/handle to a component. The handle caches the actual pointer of the object and updates it only when necessary, making access through it fairly fast. All permanent references should be handled by ComponentRef, and never by raw or smart pointers. For smaller scopes, raw pointers are preferred for fast access. Pointers and handles are convertable to one another.  
```cpp
ComponentRef<TransformComponent> Ref = GameObjectPtr->GetComponent<TransformComponent>()->GetPermanentReference();
// GetPermanentReference is optional since the types are convertable, but guarantees the correct type
```
  
## Object ownership
All gameobjects are owned and handled by the scene they are added to. The scene makes sure that the objects are deleted when not needed or become obsolete, so the user should never call delete explicitly. Gameobjects can be destroyed at any point using their Destroy function. This does not guarantee immediate destruction, but they will be disabled for the next tick and marked as deleted.
  
All components are owned by their gameobject, but their actual lifetime is handled by their own allocator. Components may never be moved to another gameobject. Components are marked as unused and after being removed from a gameobject manually or during its destruction.  
  
## Systems
System Manager only handles sound system at the moment, but all other systems such as rendering and input should be moved under it. Other systems are accessed through singletons.
  
## Gameplay programming
The game uses game state machine to track the state of the game. The users may add all required states and path conditions in the load function, where they are provided with a reference to the state machine. All scene specific game logic should be contained in these states and respective factory functions. Users may also create custom components to contain gameplay specific logic. Most logic can be bound during component and gameobject creation using functors and delegates. Almost all necessary functions are exposed as delegates the user may bind arbitrary number of functors to manage changing gameplay requirements. For example, the collision reaction logic can be easily changed by unbinding and binding functions from the delegate.

```cpp
MulticastDelegate<Direction> OnDirectionChanged; // Delegate with one parameter
MulticastDelegate<Direction>::DelegateHandle Handle = OnDirectionChanged.Bind(GameObjectPtr, [](Direction dir) {
	// Code...
});
OnDirectionChanged.Broadcast(Direction::Down);
OnDirectionChanged.Unbind(Handle);
OnDirectionChanged.UnbindAll();
```  
DelegateHandle is a container for a handle to the specific binding. The typedef is created automatically and can be used only with the delegate it was created from.  
  
## Timers
The engine contains helpers for creating and managing timers. Timers are created by specifying the time and a callback function that may be a functor, and additionally request looping. Timers can also be specified to run asynchronously, but thread safety is not guaranteed. Async timers should be used very carefully and conservatively. 
```cpp
Timer T = Time::GetInstance().SetTimerByEvent(2.f, []() {
    // Code...
});
Time::GetInstance().ClearTimer(T);
AsyncTimer AT = Time::GetInstance().LaunchAsyncTimerByEvent(1.f, []() {
    // Code...
});
```
  
## Input
Input system uses SDL inputs for keyboardand mouse and XInput for controllers. Up to four controllers may be connected in addition to the keyboard. This limit is imposed by XInput, the engine could track more.  
  
The engine uses input mappings to track user actions. In the load function, or at any other time, the user may create a new mapping, where they map an arbitrary number of keys to actions, with additional modifiers if needed. The user may then create an InputComponent, through which they can then bind actions to functions. This input component is added to the players input stack and receives the inputs after previous components have handled it. For now, input is never consumed. If the component is inactive or its input has been disabled, tha actions will not be fired.  
Together these steps make managing player inputs very easy, as the input mapping may be switched at any time to change the actions and key configurations to fire them. The mappings need to be created only once and can be set using an identifier.
```cpp
InputManager::GetInstance().MakeInputMapping("Default",
{
    {"Mute", {
        {
            Key('m')
        }, false
        }
    },
    {"Move",{
        {
            Key('w', DeviceType::Keyboard, InputMappingType::DigitalToY),
            Key('a', DeviceType::Keyboard, InputMappingType::DigitalToNegX),
            Key('s', DeviceType::Keyboard, InputMappingType::DigitalToNegY),
            Key('d', DeviceType::Keyboard, InputMappingType::DigitalToX),
            Key(Buttons::Axis::ControllerStickLeft, DeviceType::Controller, InputMappingType::Axis2D),
            Key(Buttons::Controller::DpadUp, DeviceType::Controller, InputMappingType::DigitalToY),
            Key(Buttons::Controller::DpadLeft, DeviceType::Controller, InputMappingType::DigitalToNegX),
            Key(Buttons::Controller::DpadDown, DeviceType::Controller, InputMappingType::DigitalToNegY),
            Key(Buttons::Controller::DpadRight, DeviceType::Controller, InputMappingType::DigitalToX),
        },
        true }
    },
});

InputManager::GetInstance().SetUserMapping(userID, "Default");
InputComponentPtr->BindAction("Mute", [] {
	// code...
});
```
