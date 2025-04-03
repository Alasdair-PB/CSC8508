# CSC8508
## Team 2's Group Project

### Video Demonstrations

#### Gameplay Loop
https://youtu.be/-x3eM8zoQJ0
#### Custom Editor
https://youtu.be/CDocuHMd9Sw 
#### EOS lobby system

#### Voice chat Demonstration

### Controls
-Use WASD to move forwards, backwards, left or right respectively
-Use the mouse to look around and control the camera
-Use the spacebar to jump
-Use the e key to interact with objects including picking up items
-Use the l key to toggle debug mode, showing the framerate counter

### Objectives
You must navigate through the dungeon, finding items, picking them up and bringing them back to the starting room where you drop them on the green pads to earn currency.
You can interact with the blue pads to transfer your currency from the inventory to your stored currency.
Interacting with the red pad will exit the game.

### Network
If one player starts as the server from the main menu, another player can start as client to join their game and play multiplayer.
You will be able to hear the other players through voice chat.

### Team Roles
#### Alasdair PB
* Entity Component system
* Network Initialization:: Spawn packets and connection confirmation
* Transform Packets
* NetworkComponents (event packets)
* NetworkDeltaComponents (Delta & full packets)
* Input prediction & network rollback
* Serialized Save
* Child GameObjects with save & physics support 
* Feature integration: inventoryManager & pickup items
* Feature integration: player controller
* Player Model & animations
* The Editor build mode (all)
* Feature plans:: (ECS, Network, PlayerController, Enemy states, Inventory, PushDown Menus, et al)
  
#### Kieran
* Implementing ImGui library
* ImGui integration with existing code and bug fixing
* Framerate Counter 
* Main Menu UI
* Audio Sliders
* Health Bar
* Implementing UI scaling
* Converting UI to a state based system
* Inventory UI
* Changing UI system to work with component system
* Pause Menu

#### Max
* FMOD integration & Audio Engine Instantiation
* Spatial Audio playback
* Main and secondary sound channels for volume
* Microphone recording/playback
* Microphone encoding/decoding
* Sending/receiving encoded/decoded audio packets
* Audio asset data structure
* Timer Component
* Network Timer Component
* Bank behaviour
* Exit game logic

#### Alfie
* Event system
* Scene system
* Capsule-OBB and OBB-OBB collision detection
* Spatial acceleration
* Moving gen to component structures and GenerateRoom() algorithm
* RoomPrefab component saving and prefab instance adjusting
* Core class maths: Axis object type, VectorsToQuaternion()  et al
* GameObject child Transform position and matrix calculations

#### David
* Full PS5 compatibility
* FMOD with PS5
* Enet to PS5
* ImGui ported to PS5
* Decoupling and configuration to make PS5 and windows share as much code as possible
* Opus with PS5
* Misc Component work
* OGL renderer optimisations and tweaks
* AnimationComponent & state machine

#### Ding
* preparations for the generation of dungeon
* generation of dungeon & prepare room prefab type
* DamageableComponent
* ItemComponent
* InventoryManagerComponent
* GameManagerComponent
* Prepare 3D models and model them use blender
* Create trouble for the team
* respawn component

#### Michael
* Player Component
* IState Machine for enemy
* Sight Component
* Stamina Component
* Export Assets to Meshes
* Create additional rooms and export with navmeshes
* DamageNetwork Component
* Item Weight Modifier to movement
* Tool to run multiple instances and close all on keybinds 

#### Matt
* Epic Online Services
* Lobbies, connection and UI for epic online systems. 

### Submission Known issues
Please note the issues listed were not fixed due to task priorities while finishing the project and time pressure, not due to task complexity.
#### Editor
* Loading a world file as a Pfab will cause the Editor to crash.
* Loading a Pfab file as a world will cause the Editor to crash- The default file set on load up is a World file, please do not load this as a Pfab while testing.
* Incorrect file paths are not supported causing the Editor to crash if the file asset is spelt incorrectly. 
* Not a bug, but usability note:: saving as PFab saves the object selected. If a room Pfab is loaded- please use the focus parent button to select the parent for saving- otherwise the save file will be changed to only the selected object. 

#### Game
* Health & stamina UI was not incorporated into the network infrastructure. Multiple UI elements may be overlayed causing an incorrect values to be displayed when playing with multiple clients.
* The Game Over screen does not reflect the victory state of the player and may use the same end state regardless of player success.
  
### Submission
A tag has been created for the submitted release of this project. Tag::`Submission` was the final submssion for this coursework and is the one intended for marking. While the project may continue to have changes after this time of submssion the release was submitted before the deadline and will remain unedited. 
To view the submitted version either checkout the release tag 'Submission' from main or view branch::[CSC8508 Submission](https://github.com/Alasdair-PB/CSC8508/tree/Submission-Csc8505). 
