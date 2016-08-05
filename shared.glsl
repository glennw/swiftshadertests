#ifdef WR_VERTEX_SHADER
    #define varying out

	in vec3 aPosition;
#endif

#ifdef WR_FRAGMENT_SHADER
    precision highp float;

    #define varying in

    out vec4 oFragColor;
#endif

varying vec4 vColor;
