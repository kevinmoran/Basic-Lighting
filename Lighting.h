#pragma once

struct DirectionalLight {
    vec3 dir; //NB: direction *TO* the light
    vec3 colour; 
};

struct PointLight {
    vec3 pos;
    vec3 colour;
    // float constant; //TODO: attenuation
    // float linear;
    // float quadratic; 
};

DirectionalLight g_dir_light;
PointLight g_point_light;