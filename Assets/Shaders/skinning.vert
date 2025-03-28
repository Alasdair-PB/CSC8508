#version 400

uniform mat4 modelMatrix = mat4(1.0f);
uniform mat4 viewMatrix = mat4(1.0f);
uniform mat4 projMatrix = mat4(1.0f);
uniform mat4 shadowMatrix = mat4(1.0f);
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;
uniform mat4 joints[128];

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 5) in vec4 jointWeights;
layout(location = 6) in ivec4 jointIndices;

uniform vec4 		objectColour = vec4(1,1,1,1);
uniform bool hasVertexColours = false;

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	//vec3 tangent; 
    //vec3 binormal;
} OUT;

void main(void) {	
    vec4 localPos = vec4(position, 1.0);
    vec4 skelPos = vec4(0, 0, 0, 0);

    for (int i = 0; i < 4; ++i) {
        int jointIndex = jointIndices[i];
        float jointWeight = jointWeights[i];

        skelPos += joints[jointIndex] * localPos * jointWeight;
    }
	
	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));

	OUT.shadowProj 	=  shadowMatrix * vec4 ( skelPos.xyz,1);
	OUT.worldPos 	= (modelMatrix * vec4(skelPos.xyz, 1.0)).xyz;
	OUT.normal 		= normalize ( normalMatrix * normalize ( normal ));

	OUT.texCoord	= texCoord;
	OUT.colour		= objectColour;
	
	gl_Position		= mvp * vec4(skelPos.xyz, 1.0);
	
    if(hasVertexColours) {
		OUT.colour		= objectColour * colour;
	}
	
	//vec3 viewDir = normalize (lightPos - OUT.worldPos.xyz);
	//vec4 pushVal = vec4(OUT.normal, 0) * dot(viewDir, OUT.normal );
	//OUT.shadowProj = shadowMatrix * (worldPos + pushVal);
}
