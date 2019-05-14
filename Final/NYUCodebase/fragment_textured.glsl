
uniform sampler2D diffuse;
uniform float alphaVar;
varying vec2 texCoordVar;
uniform float blackScreen;

void main() {
    vec4 t = texture2D(diffuse, texCoordVar);
    if(blackScreen == 1.0){
	gl_FragColor = vec4(0.0, 0.0, 0.0, alphaVar);
    } else {  
        gl_FragColor = t;
    }
   
    
    
}