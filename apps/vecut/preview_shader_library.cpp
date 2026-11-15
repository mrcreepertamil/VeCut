#include "preview_shader_library.h"

namespace vecut {

QString PreviewShaderLibrary::vertex_source(bool gles) {
    if (gles) return QStringLiteral(
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTexCoord;\n"
        "varying vec2 vTexCoord;\n"
        "void main(){ gl_Position=aPosition; vTexCoord=aTexCoord; }\n");
    return QStringLiteral(
        "#version 120\n"
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTexCoord;\n"
        "varying vec2 vTexCoord;\n"
        "void main(){ gl_Position=aPosition; vTexCoord=aTexCoord; }\n");
}

QString PreviewShaderLibrary::fragment_source(bool gles) {
    const QString header = gles
        ? QStringLiteral("precision mediump float;\n")
        : QStringLiteral("#version 120\n");
    return header + QStringLiteral(
        "uniform sampler2D uTexture;\n"
        "uniform int uFilter;\n"
        "varying vec2 vTexCoord;\n"
        "void main(){\n"
        " vec4 c=texture2D(uTexture,vTexCoord);\n"
        " if(uFilter==1){ float g=dot(c.rgb,vec3(0.299,0.587,0.114)); c.rgb=vec3(g); }\n"
        " else if(uFilter==2){ c.r=min(1.0,c.r*1.08+0.025); c.g*=0.99; c.b*=0.90; }\n"
        " else if(uFilter==3){ c.r*=0.90; c.g*=0.99; c.b=min(1.0,c.b*1.08+0.02); }\n"
        " else if(uFilter==4){ c.rgb=min(vec3(dot(c.rgb,vec3(0.393,0.769,0.189)),"
        "dot(c.rgb,vec3(0.349,0.686,0.168)),dot(c.rgb,vec3(0.272,0.534,0.131))),vec3(1.0)); }\n"
        " else if(uFilter==5){ float g=dot(c.rgb,vec3(0.299,0.587,0.114));"
        " c.rgb=mix(vec3(g),c.rgb,0.55)*vec3(1.08,1.00,0.88);"
        " c.rgb=(c.rgb-0.5)*1.12+0.5; }\n"
        " else if(uFilter==6){ c.rgb=vec3(1.0)-c.rgb; }\n"
        " else if(uFilter==7){ c.rgb=clamp((c.rgb-0.5)*1.35+0.5,0.0,1.0); }\n"
        " gl_FragColor=c;\n"
        "}\n");
}

} // namespace vecut
