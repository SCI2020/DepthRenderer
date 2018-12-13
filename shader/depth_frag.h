const char *depth_fragment_code =
"#version 330 core\n"

"out vec4 color;\n"
"in vec3 vertex_position;\n"

"uniform mat4 VP;\n"
"uniform int objectID;"
"uniform float near;\n"
"uniform float far;\n"

"float LinearizeDepth(float depth)\n"
"{\n"
"	float z = depth * 2.0 - 1.0;\n"
// Back to NDC 
"	return (2.0 * near * far) / (far + near - z * (far - near));\n"
"}\n"

"void main()\n"
"{\n"
// Render depth
// equally divide the length between near and far (256 pieces)
"	float depth = (LinearizeDepth(gl_FragCoord.z) - near) / (far - near);\n"
"	color = vec4(depth, depth, depth, 0.0);\n"

// Render label
"	//color.r = (((objectID & 0x18) << 3) & 0xff) / 255.0;	\n"
"	//color.g = (((objectID & 0x04) << 5) & 0xff) / 255.0;	\n"
"	//color.b = (((objectID & 0x03) << 6) & 0xff) / 255.0;	\n"
"}\n";
