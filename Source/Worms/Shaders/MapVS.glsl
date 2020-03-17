#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

// Uniform properties
uniform sampler2D heightMap;
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec2 heightMapSize;

out vec2 texcoord;
out vec3 world_position;
out vec3 world_normal;

void main()
{
	// TODO : pass v_texture_coord as output to Fragment Shader
	texcoord = v_texture_coord;
	world_position = vec3(Model * vec4(v_position, 1));

	float height = texture2D(heightMap, v_texture_coord).x/2;
	vec3 new_pos = v_position;
	new_pos.y = new_pos.y + height;

	float u = v_texture_coord.x;
	float v = v_texture_coord.y;
	
	vec2 texelSize = vec2(1.0/heightMapSize.x,1.0/heightMapSize.y);
	float heightRight = texture2D(heightMap,vec2(u+texelSize.x,v)).r;
	float heightUp = texture2D(heightMap,vec2(u,v+texelSize.y)).r;

	float Hx = height - heightRight;
	float Hz = height - heightUp;

	world_normal =normalize(mat3(Model)*normalize(vec3(Hx, 1, Hz)));

	gl_Position = Projection * View * Model * vec4(new_pos, 1.0);
}
