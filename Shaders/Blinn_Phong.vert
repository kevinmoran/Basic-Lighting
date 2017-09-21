#version 140

in vec3 vp;
in vec2 vt;
in vec3 vn;

uniform mat4 M,V,P;
uniform mat3 normal_mat;

out vec3 frag_pos_eye, frag_norm_eye;
out vec2 frag_uvs;

void main(){
    frag_pos_eye = vec3(V*M*vec4(vp, 1.0));
    frag_norm_eye = normal_mat*vn;
    frag_uvs = vt;
    gl_Position = P*vec4(frag_pos_eye, 1.0);
} 