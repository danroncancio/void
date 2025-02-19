#version 450 core

// Input

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

// Output

layout(location = 0) out vec2 fragTexCoord;

layout(set = 1, binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	fragTexCoord = inTexCoord;
}
