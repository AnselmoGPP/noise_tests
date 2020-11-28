#version 330 core

in vec4 ourColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightDir;  // not used
uniform vec3 camPos;

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);                // point light (comment this line if you use single-direction light)
    vec3 diffuse = max(dot(norm, lightDir), 0.0) * lightColor;

    // Specular lighting
    float specularStrength = 0.7;   //
    int shininess = 32;             // Smaller light shining
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Final fragment color
    vec3 color = (ambient + diffuse + specular) * ourColor.xyz;
    FragColor = vec4(color, ourColor.a);
}
