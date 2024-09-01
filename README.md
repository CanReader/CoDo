# CoDo - The Task Manager
## Overview
CToDo (or CoDo) is a modern, cross-platform task management application developed entirely in pure C using the GLFW library. Built on the foundation of cococry's leif library, which serves as the project's backbone, CToDo is designed to provide an efficient and streamlined task management experience. 
</br>The project is fully open-source, inviting developers from around the world to contribute, customize, and enhance its functionality.

## Dependencies
- ``Leif``
- ``Cglm & Glad``
- ``GLFW``
- ``Libclipboard``
- ``Stb_Image``

## Installation
There is no any additional things to installation of project. It is enough to clone the repo...   
```
git clone https://github.com/CanReader/CoDo
cd CoDo
CoDo.sln
```

## Functionalities
Upon its first launch, the app checks for the presence of the ``Data/tod.dat`` file, which is used for serialization. This file is read to populate the dashboard with any previously saved tasks. If no tasks are found, a message will appear in the designated area, stating, "No task has been assigned!"

To add a new task, simply click on "New Task," enter the task details into the text field, choose a priority level, and click "Add." The task will be automatically serialized into the .dat file and will immediately appear on the dashboard.

The app is designed with user-friendliness in mind, making it easy for anyone to manage tasks efficiently.

## Features
- ✅ Create and publish the project
- ✅ File system for serialization
- ⬜ Add date/time system
- ⬜ Convert project to cmake build system
- ⬜ Change the UI as look like the image
- ⬜ Fix serialization bug
- ⬜ Add Rest API feature
- ⬜ Save data to firebase database with REST API
- ⬜ Add shaders to create dynamic background (like there will be a glowing ball following the mouse etc.)

## Images
![CoDoIMG](https://github.com/user-attachments/assets/c70b7f37-d807-4b5f-b2b2-341d78a6a62f)
