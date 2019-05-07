#version 330 core

out vec4 FragColor;

in vec3 outColor;
in vec3 normal;
in vec3 FragPos;

// uniform sampler2D texture1;
uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess; // 32

void main()
{
	// FragColor = texture(texture1, TexCoord);
	// ambient
    // float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    // float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * outColor;
    FragColor = vec4(result, 1.0);
}