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
	int horizontalFrames;
	int currentFrame;
	vec4 modColor;
} cuo;


vec2 frameCoord = fragTexCoord;


void main()
{
	// Calculate texture frame on sprite atlas

	float xStep = 1.0 / cuo.horizontalFrames;
	float xLocalUV = fragTexCoord.x * xStep;
	frameCoord.x = xLocalUV + xStep * cuo.currentFrame;

	//

	outColor = cuo.modColor * texture(texSampler, frameCoord);
}
