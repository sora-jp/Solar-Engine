# Todo

- Time class in src/core for deltatime and more
- Inputs (InputProviders registering to a global Input class)
- Allow user control of PSO description (HACKY FIX DONE, still need to use #pragma for this tho)
- Custom HLSL preprocessor to allow for CG-like code (shaderlab)
- Tiered cbuffers? (PerScene, PerCamera, PerObject)
- Default hlsl include for cbuffer defs + other misc stuff
- SHORTEN THE GODDAMN COMPILATION TIMES AAAAAAAAAAAH
- Texture loading
- Remove wierd shit from GraphicsSubsystem
- Maybe make DiligentContext purely static?
- Figure out wtf is wrong with relative paths in Assimp
- Wrap buffer creation API
- Higher level rendering API (DrawRenderer?) (SORTA done through RenderPipelines, but still)
- Split DiligentInit.cpp into multiple files, one for init / deinit, one for draw-related commands
- Refactor ImGuiDiligentRenderer to use DiligentContext instead of raw IDeviceContext
- Threaded rendering?
- Cleanup literally all the code in src/graphics, and look through src/core