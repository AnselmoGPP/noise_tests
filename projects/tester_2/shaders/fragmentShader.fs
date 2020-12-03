#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightDir;  // not used
uniform vec3 camPos;

uniform sampler2D texture1;  // more: sampler1D, sampler3D
//uniform sampler2D texture2;

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);                // point light (comment this line if you use directional light)
    vec3 diffuse = max(dot(norm, lightDir), 0.0) * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    int shininess = 32;
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0) * texture(texture1, TexCoord);
    //vec3 color = (ambient + diffuse + specular) * objectColor;
    //FragColor = vec4(color, 1.0f);

    //FragColor = vec4(ourColor, 1.0f);
    //FragColor = texture(texture1, TexCoord);
    //FragColor = texture(texture1, TexCoord) * vec4(ourColor, 1.0);
    //FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.5);
    //FragColor = vec4(lightColor * objectColor, 1.0);
}

