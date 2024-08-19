#version 460 core

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_Normal;

// Set 0
layout(set = 0, binding = 1) uniform sampler2D u_Albedo;

void main()
{
    o_Colour = texture(u_Albedo, v_TexCoord);
}