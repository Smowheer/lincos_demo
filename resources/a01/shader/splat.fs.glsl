#version 330
  
layout(location = 0)  out vec4 out0; // color 

in vec4 c;

uniform uvec2 resolution;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D position;

uniform float radius;
uniform vec3 splatColor;

void main() 
{ 
	vec2 tc = gl_FragCoord.xy / resolution;
	// TODO:
	// - gbuffer position in splat radius?
	// - diffuse shading
	// - distance attenuation

	out0 = vec4(splatColor, 1.0);
}
