#version 450 core

// Output

layout(location = 0) out vec4 outColor;


// Uniforms

layout(set = 3, binding = 0) uniform UniformBufferObject
{
	float time;
	vec4 modColor;
} cuo;

void main()
{
	outColor = cuo.modColor;
}

