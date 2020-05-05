#version 330

// this is is a generic Phong shading model shader

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

uniform vec4 lightPosition;

in vec4 vertex;
in vec3 vertexNormal;
in vec4 vertexColour;

out vec3 normalFrag;
out vec3 eyeFrag;
out vec3 lightDirFrag;
out vec4 colourFrag;

void main ()
{
    vec4 pos = mvMatrix * vertex;

    normalFrag = normalize(normalMatrix * vertexNormal);
    lightDirFrag = vec3(lightPosition - pos);
    eyeFrag = vec3(-pos);
    colourFrag = vertexColour;

    gl_Position = mvpMatrix * vertex;
}

