#version 330 core

layout (location = 0) out vec4 FragColor;


in vec2 TexCoord;

uniform sampler2D uImage;
uniform bool uHorizontal;

const float weights[5] = float[](
    0.227027,
    0.194594,
    0.121621,
    0.054054,
    0.016216
);


void main() {

    
    vec2 texelSize =
        1.0 /
        vec2(textureSize(uImage, 0));


    vec3 result =
        texture(
            uImage,
            TexCoord
        ).rgb * weights[0];


    for (int i = 1; i < 5; ++i) {
        
        vec2 offset;

        if (uHorizontal) {
            
            offset = vec2(
                texelSize.x * float(i),
                0.0
            );
   

        }else {
        
            offset = vec2(
                0.0,
                texelSize.y * float(i)
            );
        
        }

        result += texture(
            uImage,
            TexCoord + offset
        ).rgb * weights[i];

        result += texture(
            uImage,
            TexCoord - offset
        ).rgb * weights[i];

        
    }


    FragColor = vec4(result, 1.0);

}