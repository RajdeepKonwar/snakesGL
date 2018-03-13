#version 330 core

in vec3 FragCoord;
in vec4 viewSpace;

out vec4 FragColor;

void main() {
  //FragColor = vec4( 0.7f, 0.8f, 0.0f , 1.0 );
  
  vec3 distanceVector = vec3(viewSpace) - vec3(0.0f, -3.5f, 2.5f);
  float dist = length(distanceVector);
  
  vec4 tileColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  
  vec4 fogColor = vec4(0.3f, 0.3f, 0.3f, 1.0f);
  
  float minFogDist = 5.0f;
  float maxFogDist = 25.0f;
    
  float fog_factor = (maxFogDist - dist) / (maxFogDist - minFogDist);
    
  fog_factor = clamp(fog_factor, 0.0f, 1.0f);
  
  FragColor = mix(fogColor, tileColor, fog_factor);
    
}
