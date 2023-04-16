Developed with Unreal Engine 5. 

Unfinished and currently worked on Unreal Engine 5 project written in C++ and blueprints.

Controls: WASD = movement, Q = quick turn, LMB = Grapplinghook, Shift = Sprint, LCTRL = crouch

Crouching while sprinting will slide

Bouncing ball on the top of the map is a grappling pickup. End idea is you can only use grappling hook if you grab that pick up but for testing purposes its enabled by default

Jumping, sliding or pressing the grappling hook button while the grappling hook is active will cancel the hook. Can be used to gain momentum or get through tight areas.

C++ code is in Source/Grounders/Private/Players. Rest of the coding is done in blueprints.

Blueprint codes can be found in Content/GameModes/BP_GameModes, Content/GameStates/TagGameState, Content/GrapplingHook/GrapplingComponent, Content/GrapplingHook/BP_GrapplingHook, Content/HUD/Hud2, Content/HUD/Winner, Content/Pickups/GrapplingHookPickup, Content/PlayersOrCharacters/BP_Players, 
