#version 460 core

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos;

void main()
{
    const float ambientStrength = 0.1f;
    const vec3 ambient = ambientStrength * lightColor;

    const vec3 norm = normalize(Normal);
    const vec3 lightDir = normalize(lightPos - FragPos);

    const float diff = max(dot(norm, lightDir), 0.0f);
    const vec3 diffuse = diff * lightColor;

    const float specularStrength = 1.f;
    const vec3 viewDir = normalize(viewPos - FragPos);
    const vec3 reflectDir = reflect(-lightDir, norm);

    const float spec = pow(max(dot(viewDir, reflectDir), 0.f), 32.f);
    const vec3 specular = specularStrength * spec * lightColor;

    const vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}