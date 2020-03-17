#version 330
in vec2 texcoord;
uniform sampler2D heightMap;
uniform sampler2D texture;

// TODO: get color value from vertex shader
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position1;
uniform vec3 light_position2;
uniform vec3 eye_position;
uniform float light_angle;

float material_kd = 2.0;
float material_ks = 0.1;
int material_shininess = 1000;

out vec4 out_color;

void main()
{
	// TODO : calculate the out_color using the texture2D() function
	out_color = texture2D(texture, texcoord*10);
	float height = texture2D(heightMap, texcoord).r;

	vec3 N = normalize(world_normal);
	vec3 V = normalize(eye_position - world_position);
	vec3 L1 = normalize(light_position1 - world_position);
	vec3 H1 = normalize(L1 + V);
	vec3 L2 = normalize(light_position2 - world_position);
	vec3 H2 = normalize(L2 + V);

	float dist1 = distance(light_position1, world_position);
	float dist2 = distance(light_position2, world_position);

	float ambient_light = 0.4;
	float diffuse_light1 =  material_kd * max(dot(N,L1), 0);
	float specular_light1 =  material_ks * pow(max(dot(N, H1), 0), material_shininess);
	float diffuse_light2 =  material_kd * max(dot(N,L2), 0);
	float specular_light2 =  material_ks * pow(max(dot(N, H2), 0), material_shininess);

	float cut_off = light_angle;
	float spot_light_limit = cos(cut_off);
	float spot_light1 = dot(-L1, light_direction);
	float linear_att1 = (spot_light1 - spot_light_limit) / (1 - spot_light_limit);
	float light_att_factor1 = pow(linear_att1, 1);
	float spot_light2 = dot(-L2, light_direction);
	float linear_att2 = (spot_light2 - spot_light_limit) / (1 - spot_light_limit);
	float light_att_factor2 = pow(linear_att2, 1);

	float light1, light2;

	if (spot_light1 > spot_light_limit) {
		light1 = ambient_light +  light_att_factor1 * 1/(pow(dist1,2) + 1) * (diffuse_light1 + specular_light1);
	} else { 
		light1 = ambient_light;
	}
	
	if (spot_light2 > spot_light_limit) {
		light2 = ambient_light +  light_att_factor2 * 1/(pow(dist2,2) + 1) * (diffuse_light2 + specular_light2);
	} else { 
		light2 = ambient_light;
	}
	
	vec4 snow_color = vec4(1);
	out_color = mix(out_color, snow_color, height) * max(light1, light2);
	
}