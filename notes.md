# Notes

## Low Level Game Systems
Stuff that makes the game possible but isn't core engine necessarily

1. Player - the representation of all state associated with a player that isn't necessarily the character
2. Character - the player's manifestation in the world, with a model, health, animations, hitbox, etc
3. CharacterController - the controller of the character, which tracks character state and transforms user input into motion
4. Tool - Equipped object by the Character

## Planned (Incomplete) Core Engine Features

### Rendering
- Physics Debug Rendering
- Light Emitters/Lighting Volumes
- Shadows
- Transparency
- (Potential) SSAO
- Translucency support
- Ocean Rendering V2
- (Potential) FXAA
- Shader Compilation/SPIR-V support
- Preetham model sky
- Volumetric clouds

### Physics

### Networking
- Networked Physics
- Networked Projectiles

### Animation
- Animation weights/priority layers
- AnimationTrack holding configuration/state data separate from the Animation (aka Keyframe Sequence)

### Resource
- JSON asset loading scripts
- Asynchronous asset loading
- Shader file encoding/decoding

### Other/Misc
- UI
- Lua scripting support
- Multithreading and Asynchronous Programming
- Event emission/handling
- Input consumption

## Neat Ideas

### HDR Gaussian Blur Mipmapped Cubemap
> Godot does a neat trick regarding blur, btw: during regular render of the 3D engine output pass, the rendered result is blurred and progressive levels of blur are stored in the mipmap levels of the HDR output texture
> this makes it a lot easier to do things like depth-of-field and certain kinds of bloom for the final screen output during the postprocessing phase
> as a bonus, you don't need to tap a separate blurred render texture; you can just use regular LOD sampling  
-thinwire 2020

### Instanced Animation Rendering
The Idea: Have a (double buffered?) RGBA_32F texture of size (4 * MAX_JOINTS) x (MAX_RIGS), where every 4 row pixels are one mat4 joint transform.
These will be updated in a shader pass based on input keyframe data (UBO? SSBO?) and time data.
The main instanced animation shader will read from the texture based on the instance ID

### "Sort-based draw call bucketing"
https://realtimecollisiondetection.net/blog/?p=86
Use a 64 bit ID to aggregate draw calls and sort them into full order. Currently not entirely useful/feasible but a neat idea.

## Resources/Sources
I need to fill this out.

### Rendering
- https://learnopengl.com/
- https://www.khronos.org/opengl/wiki/SPIR-V

### Physics

### Networking

### Other/Misc