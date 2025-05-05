# SquishableSponge
OpenGL Visualization where you can squeeze water out of a sponge

It's been a while since I've done any graphics programming, and I want to get back into it.  Squishable Sponge is my first attempt at implementing a particle system from scratch.  Along the way, going through learnopengl.com and the OpenGL Red Book, I kept finding things that I wanted to incorporate into a project, and they seemed to fit nicely together here.  Notably, the OpenGL code here is very concrete, with almost no abstractions.  While production-worthy code likely would abstract away the internals of OpenGL, I wanted to remain explicit here to ensure that I build an internal understanding of what OpenGL needs to do in order to render the project to the screen, instead of just throwing it into a wrapper and immediately forgetting about it.

Everything in the repository is my own code, with the third party dependencies being GLFW, GLAD, stb_image, and GLM.

A video of the project running can be found at https://nicholasmirchandani.github.io/

With this project, I:
* Rendered a cube in 3D space with a perspective view, ensuring its normals properly faced outwards to enable backface culling.
* Generated a custom sponge texture in GIMP using a flat texture, some multiply layers of noise, and a fuzzy border.
* Used stb_image to load in the custom Sponge texture, and apply it to the Cube as a Cube Map.
* Controlled the position of the cube such that the mouse points to the center of the cube via matrix transforms with GLM
* Used Multiple VertexArrays and Buffer objects for the various effects employed, keeping things relatively organized.
* Used adjustable uniforms and a fragment shader to represent a controllable water level (with some simple wave math to make it more interesting than a flat translucent line)
* Built my own Particle System, rendering up to 999 particles at a given time.
* Used simple contextual logic to cause particles to emit when dunking the sponge into the water, as well as squeezing it while holding water.
* Ensured that resizing the screen to other standard resolutions still results in a pleasing visualization, instead of largely resolution dependent speed+sizes.
* Ensured that particles bounce off the walls of the screen.

All in all, I'm relatively pleased with how this came out.
