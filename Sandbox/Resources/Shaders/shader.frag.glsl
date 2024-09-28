#version 460 core
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec4 v_Colour;

// Note: underlying type is uint16_t, so will never exceed this value.
layout(location = 3) flat in uint v_TextureID;

layout (set = 0, binding = 1) uniform sampler2D u_Textures[];

void main()
{
    o_Colour = v_Colour * texture(u_Textures[v_TextureID], v_TexCoord);
}