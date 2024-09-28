#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Colour;
layout(location = 3) in uint a_TextureID;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out vec4 v_Colour;
layout(location = 3) flat out uint v_TextureID;

layout(std140, set = 0, binding = 0) uniform CameraSettings
{
    mat4 View;
    mat4 Projection;
} u_Camera;

void main()
{
    v_Position = a_Position;
    v_TexCoord = a_TexCoord;
    v_Colour = a_Colour;
    v_TextureID = a_TextureID;

    gl_Position = u_Camera.Projection * u_Camera.View * vec4(a_Position, 1.0);
}