#version 140

struct Material {
    sampler2D diffuse_map;
    //TODO: Use textures for ambient and specular light
    //TODO: Normal mapping
    // sampler2D amb_occ_map;
    // sampler2D normal_map;
    // sampler2D spec_map;
    // float shininess;
};

struct DirectionalLight {
    vec3 dir_eye; //NB: direction *TO* the light
    vec3 colour;
};

struct PointLight {
    vec3 pos_eye;
    vec3 colour;
    float constant;
    float linear;
    float quadratic; 
};

//Bundling these so we only have to read from a texture once
struct FragLightingParams {
    vec3 diffuse_colour;
    vec3 pos_eye, norm_eye;
    float k_ambient, k_specular; //these are multiplied by the diffuse colour
    float shininess; //exponent for specularity calculation
};

//in vec2 tex_coords;
in vec3 frag_pos_eye, frag_norm_eye;
in vec2 frag_uvs;

uniform mat4 V;

//Material
uniform Material material;
float ambient_strength = 0.4;
float specular_strength = 0.9;
float specular_exponent = 100;

//Lights
//TODO: Support multiple lights!
uniform DirectionalLight dir_light;
uniform PointLight point_light;

out vec4 frag_colour;

vec3 calculate_directional_light(DirectionalLight light, FragLightingParams frag_params);
vec3 calculate_point_light(PointLight light, FragLightingParams frag_params);

void main(){
    //Read material properties
    FragLightingParams frag_params;
    frag_params.diffuse_colour = texture(material.diffuse_map, frag_uvs).xyz;
    frag_params.pos_eye        = frag_pos_eye;
    frag_params.norm_eye       = frag_norm_eye;
    frag_params.k_ambient      = ambient_strength;
    frag_params.k_specular     = specular_strength;
    frag_params.shininess      = specular_exponent;

    vec3 light_intensity = vec3(0.0);
    light_intensity += calculate_directional_light(dir_light, frag_params);
    light_intensity += calculate_point_light(point_light, frag_params);

    frag_colour = vec4(light_intensity, 1.0);
} 

vec3 calculate_directional_light(DirectionalLight light, FragLightingParams frag_params){
    vec3 i_ambient = frag_params.k_ambient * light.colour;

    float diffuse_factor = max(0.0, dot(frag_params.norm_eye, light.dir_eye));
    vec3  i_diffuse = diffuse_factor * light.colour;

    vec3  frag_to_cam_dir = normalize(-frag_params.pos_eye);
    vec3  half_way_eye    = normalize(frag_to_cam_dir + light.dir_eye);
    float specular_factor = max(0.0, dot(half_way_eye, frag_params.norm_eye));
    vec3  i_specular      = frag_params.k_specular * pow(specular_factor, 2*frag_params.shininess) * light.colour;

    vec3  result = (i_ambient + i_diffuse + i_specular) * frag_params.diffuse_colour;
    return result;
}

vec3 calculate_point_light(PointLight light, FragLightingParams frag_params){
    vec3 i_ambient = frag_params.k_ambient * light.colour;
    
    vec3 light_dir_eye   = normalize(light.pos_eye - frag_params.pos_eye);
    float diffuse_factor = max(0.0, dot(frag_params.norm_eye, light_dir_eye));
    vec3 i_diffuse       = diffuse_factor * light.colour;

    vec3 frag_to_cam_dir  = normalize(-frag_params.pos_eye);
    vec3 half_way_eye     = normalize(frag_to_cam_dir + light_dir_eye);
    float specular_factor = max(0.0, dot(half_way_eye, frag_params.norm_eye));
    vec3 i_specular       = frag_params.k_specular * pow(specular_factor, 2*frag_params.shininess) * light.colour;  

    vec3 result = (i_ambient + i_diffuse + i_specular) * frag_params.diffuse_colour;
    return result;
}