#include "preview_shader_library.h"

namespace vecut {

QString PreviewShaderLibrary::vertex_source(bool gles) {
    if (gles) return QStringLiteral(
        "attribute vec4 aPosition; attribute vec2 aTexCoord; varying vec2 vTexCoord;"
        "void main(){ gl_Position=aPosition; vTexCoord=aTexCoord; }");
    return QStringLiteral(
        "#version 120\nattribute vec4 aPosition; attribute vec2 aTexCoord;"
        "varying vec2 vTexCoord; void main(){ gl_Position=aPosition; vTexCoord=aTexCoord; }");
}

QString PreviewShaderLibrary::fragment_source(bool gles) {
    const QString header = gles ? QStringLiteral("precision mediump float;\n")
                                : QStringLiteral("#version 120\n");
    return header + QStringLiteral(
        "uniform sampler2D uTexture; uniform int uFilter;"
        "uniform vec2 uContentScale; uniform float uOpacity; uniform float uZoom;"
        "uniform float uRotation; uniform vec2 uPan; uniform float uReveal;"
        "uniform float uSlide; uniform int uWipeDirection; varying vec2 vTexCoord;"
        "void main(){"
        " vec2 screenUv=vTexCoord;"
        " if(uWipeDirection==1 && screenUv.x>uReveal) discard;"
        " if(uWipeDirection==2 && screenUv.x<1.0-uReveal) discard;"
        " vec2 uv=(screenUv-vec2(0.5))/uContentScale+vec2(0.5);"
        " if(uv.x<0.0||uv.x>1.0||uv.y<0.0||uv.y>1.0) discard;"
        " uv.x-=uSlide; uv=(uv-vec2(0.5))/max(uZoom,0.001);"
        " float cs=cos(uRotation), sn=sin(uRotation);"
        " uv=mat2(cs,-sn,sn,cs)* (uv-vec2(0.5))+vec2(0.5)-uPan;"
        " if(uv.x<0.0||uv.x>1.0||uv.y<0.0||uv.y>1.0) discard;"
        " vec4 c=texture2D(uTexture,uv); c.a*=uOpacity;"
        " if(uFilter==1){ float g=dot(c.rgb,vec3(0.299,0.587,0.114)); c.rgb=vec3(g); }"
        " else if(uFilter==2){ c.r=min(1.0,c.r*1.08+0.025); c.g*=0.99; c.b*=0.90; }"
        " else if(uFilter==3){ c.r*=0.90; c.g*=0.99; c.b=min(1.0,c.b*1.08+0.02); }"
        " else if(uFilter==4){ c.rgb=min(vec3(dot(c.rgb,vec3(0.393,0.769,0.189)),"
        "dot(c.rgb,vec3(0.349,0.686,0.168)),dot(c.rgb,vec3(0.272,0.534,0.131))),vec3(1.0)); }"
        " else if(uFilter==5){ float g=dot(c.rgb,vec3(0.299,0.587,0.114));"
        " c.rgb=mix(vec3(g),c.rgb,0.55)*vec3(1.08,1.00,0.88);"
        " c.rgb=(c.rgb-0.5)*1.12+0.5; }"
        " else if(uFilter==6){ c.rgb=vec3(1.0)-c.rgb; }"
        " else if(uFilter==7){ c.rgb=clamp((c.rgb-0.5)*1.35+0.5,0.0,1.0); }"
        " gl_FragColor=c; }");
}

} // namespace vecut
