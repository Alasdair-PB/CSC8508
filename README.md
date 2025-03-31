# CSC8508
## Team 2's Group Project

### Branch history
This branch Submission-CSC8508 was created as an easy access way to pull the version submitted when the tag was created. While this branch was created after the submission, only the read me file has changed since the date of submission. 

### Submission
A tag has been created for the submitted release of this project. Tag::`Submission1.0` was the final submssion for this coursework and is the one intended for marking. While the project may continue to have changes after this time of submssion the release was submitted before the deadline and will remain unedited. 

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

### Video Demonstrations

#### Gameplay Loop
https://youtu.be/-x3eM8zoQJ0
#### Custom Editor
https://youtu.be/CDocuHMd9Sw 
#### EOS lobby system

#### Voice chat Demonstration

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
