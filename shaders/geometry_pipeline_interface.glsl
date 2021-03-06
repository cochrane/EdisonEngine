layout(location=11) IN_OUT GeometryPipelineInterface {
    vec2 texCoord;
    vec3 color;
    flat float texIndex;
    vec3 vertexPos;
    vec3 vertexPosLight1;
    vec3 vertexPosLight2;
    vec3 vertexPosLight3;
    #ifdef WATER
    vec3 vertexPosWorld;
    #endif
    vec3 normal;
    vec3 ssaoNormal;
} gpi;
