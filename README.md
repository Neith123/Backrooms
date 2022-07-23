# Backrooms

## File structure

| Files                                             | Purpose                                                                               |
|---------------------------------------------------|---------------------------------------------------------------------------------------|
| backrooms_camera.h backrooms_camera.cpp           | The different camera systems used throughout the engine.                              |
| backrooms_audio.h                                 | Contains type definitions and function for the audio subsystem of the engine.         |
| backrooms_common.h                                | Contains general type definitions for all the engine.                                 |
| backrooms_forward.h backrooms_forward.cpp         | The forward pass implementation.                                                      |
| backrooms_frame_graph.h backrooms_frame_graph.cpp | A frame graph implementation. Not really a graph though.                              |
| backrooms_frame_graph_types.h                     | Contains types for the frame graph implementation.                                    |
| backrooms_entity.h backrooms_entity.cpp           | Contains types and functions for the entity system.                                   |
| backrooms_input.h backrooms_input.cpp             | Contains types and functions for the input subsystem of the engine.                   |
| backrooms_logger.h backrooms_logger.cpp           | Contains a logging implementation.                                                    |
| backrooms_model.h backrooms_model.cpp             | Contains a GLTF loader.                                                               |
| backrooms_rhi.h                                   | Contains the interface for the RHI.                                                   |
| backrooms_rhi_d3d11.cpp                           | The D3D11 implementation of the RHI.                                                  |
| backrooms_platform.h                              | Contains the interface for the platform system.                                       |
| backrooms_win32.cpp                               | Contains the Windows entry point and the Win32 implementation of the platform system. |
| backrooms.h backrooms.cpp                         | Contains functions and definitions that holds all the data about the game.            |

## Dependencies

- [cgltf](https://github.com/jkuhlmann/cgltf)
- [dr_libs](https://github.com/mackron/dr_libs)
- [HandmadeMath](https://github.com/HandmadeMath/Handmade-Math)
- [stb](https://github.com/nothings/stb)
- [ImGui](https://github.com/ocornut/imgui)