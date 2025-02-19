#version 450 core

// Input

layout(location = 0) in vec2 fragTexCoord;


// Output

layout(location = 0) out vec4 outColor;


// Uniforms

layout(set = 2, binding = 0) uniform sampler2D texSampler;

layout(set = 3, binding = 0) uniform UniformBufferObject
{
	float time;
	vec4 modColor;
} cuo;


void main()
{
	outColor = cuo.modColor * texture(texSampler, fragTexCoord);
}
