#version 330 core

out vec2 vScreenUV;

void main() {

	vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );


    vec2 pos = positions[gl_VertexID];

    vScreenUV = pos * 0.5 + 0.5;


    gl_Position = vec4(pos, 0.0, 1.0);

}