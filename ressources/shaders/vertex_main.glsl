#version 440

uniform mat4 projMatrix;
out vec2 uv; //finicky output
void main() {
    vec2 pos;
    vec4 st = vec4(-1.0, 1.0, -1.0, 1.0);  //const values
    if (gl_VertexID == 0)       { pos.x = st.x; pos.y = st.z; uv.x = 0; uv.y = 0;}
    else if (gl_VertexID == 1)  { pos.x = st.x; pos.y = st.w; uv.x = 0; uv.y = 1;}
    else if (gl_VertexID == 2)  { pos.x = st.y; pos.y = st.z; uv.x = 1; uv.y = 0;}
    else if (gl_VertexID == 3)  { pos.x = st.y; pos.y = st.w; uv.x = 1; uv.y = 1;}
    gl_Position = vec4( pos, 0.0, 1.0 );
}
