#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH
		#define highp mediump
	#endif
	precision highp float;
#endif

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization
in vec3 norm;
in vec4 epos;

// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
//uniform sampler2D TEX;
uniform mat4 view_matrix;
uniform vec4 light_position, Ia, Id, Is;
uniform vec4 Ka, Kd, Ks;
uniform float shininess;
uniform bool mode_text;
uniform vec4 color;

vec4 phong( vec3 l, vec3 n, vec3 h, vec4 Kd )
{
	vec4 Ira = Ka*Ia;									// ambient reflection
	vec4 Ird = max(Kd*dot(l,n)*Id,0.0);					// diffuse reflection
	vec4 Irs = max(Ks*pow(dot(h,n),shininess)*Is,0.0);	// specular reflection
	return Ira + Ird + Irs;
}

void main()
{
	vec4 lpos = view_matrix*light_position;

	vec3 n = normalize(norm);
	vec3 p = epos.xyz;
	vec3 l = normalize(lpos.xyz-(lpos.a==0.0?vec3(0):p));
	vec3 v = normalize(-p);
	vec3 h = normalize(l+v);

	//vec4 iKd = texture(TEX,tc);

	//if(mode==0)		fragColor = texture(TEX,tc);
	//else			fragColor = phong( l, n, h, iKd );
	if(mode_text)	fragColor = phong(l, n, h, color);
	else			fragColor = phong(l, n, h, Kd);
}