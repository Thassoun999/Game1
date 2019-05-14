
uniform sampler2D diffuse;
uniform float alphaVar;
varying vec2 texCoordVar;


void main() {
    vec4 t = texture2D(diffuse, texCoordVar);
    t.a = t.a * alphaVar;
    gl_FragColor = t;
    
}