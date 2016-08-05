struct Item {
    vec4 color;
};

layout(std140) uniform Items {
    Item items[8];
};

void main() {
    vColor = items[gl_InstanceID].color;
    gl_Position = vec4(aPosition, 1);
}