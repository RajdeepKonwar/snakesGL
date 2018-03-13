#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 modelView;

out vec3 FragCoord;
out vec4 viewSpace;

void main() {
  gl_Position = projection * modelView * vec4( aPos, 1.0 );
  viewSpace = modelView * vec4( aPos, 1.0 );
  FragCoord   = aPos;
}
