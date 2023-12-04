#version 330 core

in vec3 position;
in vec3 normal;

out vec4 fragColor;

uniform vec4 k_a;
uniform vec4 k_d;
uniform vec4 k_s;

uniform vec3 lightDirection;

uniform vec3 cameraPosition;
uniform float shininess;

float falloff(float x, float theta_inner, float theta_outer) {
    float z = (x - theta_inner) / (theta_outer - theta_inner);
    float falloff = -2 * pow(z, 3) + 3 * pow(z, 2);

    if (x <= theta_inner) return 1;
    else if (x <= theta_outer) return 1 - falloff;
    else return 0;
}

float attenuation(vec3 function, float distance) {
    return min(1, 1 / (function.x + distance * function.y + distance * distance * function.z));
}

void main() {
    fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    fragColor += k_a;

    fragColor += k_d * min(max(dot(normalize(normal), -lightDirection), 0.0f), 1.0f);

    vec3 viewDirection = normalize(cameraPosition - position);
    vec3 reflectedLight = reflect(lightDirection, normalize(normal));
    float specularTerm = min(max(dot(viewDirection, reflectedLight), 0.0), 1.0f);
    if (shininess > 0) specularTerm = pow(specularTerm, shininess);
    else specularTerm = 1;
    fragColor += k_s * specularTerm;
}