#include "Worms.h"

#include <vector>
#include <string>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

Worms::Worms()
{
}

Worms::~Worms()
{
}

void Worms::Init()
{
	const string textureLoc = "Source/Worms/Textures/";
	srand(time(NULL));


	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Loading map
	heightPixels = stbi_load((textureLoc + "heightmap.png").c_str(), &width, &height, &channels, STBI_grey);

	player1_pos.x = ((float) (rand() % (width/2) + width/4)) / (width - 1) * 4;
	player1_pos.z = ((float) (rand() % (height/2) + height/4)) / (height - 1) * 4;
	player1_pos.y = GetHeightOnMapPoint(player1_pos.x, player1_pos.z);

	player2_pos.x = ((float)(rand() % (width / 2) + width / 4)) / (width - 1) * 4;
	player2_pos.z = ((float)(rand() % (height / 2) + height / 4)) / (height - 1) * 4;
	player2_pos.y = GetHeightOnMapPoint(player2_pos.x, player2_pos.z);

	camera_p1 = new Laborator::Camera(player1_pos + glm::vec3(0.1f), player1_pos, glm::vec3(0, 1, 0));
	camera_p1->distanceToTarget = glm::distance(player1_pos + glm::vec3(0.1f), player1_pos);
	camera_p1->TranslateUpword(0.1f);

	camera_p2 = new Laborator::Camera(player2_pos + glm::vec3(0.1f), player2_pos, glm::vec3(0, 1, 0));
	camera_p2->distanceToTarget = glm::distance(player1_pos + glm::vec3(0.1f), player1_pos);
	camera_p2->TranslateUpword(0.1f);
	
	camera_global = new Laborator::Camera();
	camera_global->Set(glm::vec3(3,1,3), glm::vec3(2,0,2), glm::vec3(0, 1, 0));
	camera_projectile = new Laborator::Camera();

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "ground.jpg").c_str(), GL_REPEAT);
		mapTextures["Ground"] = texture;
	}
	
	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("worm_body");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Characters/Worm", "Worm_Body.obj");
		meshes[mesh->GetMeshID()] = mesh;

		Texture2D* texture = new Texture2D();
		texture->Load2D((RESOURCE_PATH::MODELS + "Characters/Worm/Textures/Body.png").c_str(), GL_REPEAT);
		mapTextures["worm_body"] = texture;
	}

	{
		Mesh* mesh = new Mesh("worm_eyes");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Characters/Worm", "Worm_Eyes.obj");
		meshes[mesh->GetMeshID()] = mesh;

		Texture2D* texture = new Texture2D();
		texture->Load2D((RESOURCE_PATH::MODELS + "Characters/Worm/Textures/Eyes.png").c_str(), GL_REPEAT);
		mapTextures["worm_eyes"] = texture;
	}

	{
		Mesh* mesh = new Mesh("worm_hands");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Characters/Worm", "Worm_Hands.obj");
		meshes[mesh->GetMeshID()] = mesh;

		Texture2D* texture = new Texture2D();
		texture->Load2D((RESOURCE_PATH::MODELS + "Characters/Worm/Textures/Hands.png").c_str(), GL_REPEAT);
		mapTextures["worm_hands"] = texture;
	}

	{
		Mesh* mesh = new Mesh("bazooka");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Characters/Worm", "Bazooka.obj");
		meshes[mesh->GetMeshID()] = mesh;

		Texture2D* texture = new Texture2D();
		texture->Load2D((RESOURCE_PATH::MODELS + "Characters/Worm/Textures/green.jpg").c_str(), GL_REPEAT);
		mapTextures["bazooka"] = texture;
	}

	{
		Mesh* mesh = new Mesh("cube");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		vector<std::string> faces
		{
			textureLoc + "mnight_rt.tga",
			textureLoc + "mnight_lf.tga",
			textureLoc + "mnight_up.tga",
			textureLoc + "mnight_dn.tga",
			textureLoc + "mnight_bk.tga",
			textureLoc + "mnight_ft.tga",
		};
		cubemapTexture = loadCubemap(faces);
	}

	// Create a simple quad
	{
		vector<glm::vec3> vertices;
		vector<glm::vec3> normals;
		vector<glm::vec2> textureCoords;
		vector<unsigned short> indices;
		
		for (int z = 0; z < height; z++) {
			for (int x = 0; x < width; x++) {
				vertices.push_back(glm::vec3((float) x / (width-1) * 4, 0, (float)z / (height-1) * 4));
				normals.push_back(glm::vec3(1.0, 1.0, 1.0));

				textureCoords.push_back(glm::vec2((float)x / (width-1), (float)z / (height-1)));

				if (z > 0 && x < width - 1) {
					indices.push_back(z * width + x);
					indices.push_back((z - 1) * width + x + 1);
					indices.push_back((z - 1) * width + x);

					indices.push_back(z * width + x);
					indices.push_back(z * width + x + 1);
					indices.push_back((z - 1) * width + x + 1);
				}
			}
		}

		Mesh* mesh = new Mesh("square");
		mesh->InitFromData(vertices, normals, textureCoords, indices);
		meshes[mesh->GetMeshID()] = mesh;
	}

	// Create a shader program for drawing face polygon with the color of the normal
	{
		Shader* shader = new Shader("MapShader");
		shader->AddShader("Source/Worms/Shaders/MapVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Worms/Shaders/MapFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("BasicShader");
		shader->AddShader("Source/Worms/Shaders/BasicVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Worms/Shaders/BasicFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("SkyboxShader");
		shader->AddShader("Source/Worms/Shaders/SkyboxVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Worms/Shaders/SkyboxFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

void Worms::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0.19, 0.16, 0.29, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);

	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);
}

void Worms::Update(float deltaTimeSeconds)
{
	glLineWidth(3);
	glPointSize(5);
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);


	glm::mat4 modelMatrix = glm::mat4(1);
	RenderMap(modelMatrix, heightPixels, mapTextures["Ground"]);
	
	modelMatrix = glm::translate(modelMatrix, glm::vec3(2, 0, 2));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(4, 4, 4));
	glCullFace(GL_FRONT);
	RenderCubemap(modelMatrix);
	glCullFace(GL_BACK);

	modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, player1_pos);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.005f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f, 0.01f, 0.01f));
	modelMatrix = glm::rotate(modelMatrix, camera_p1->GetCameraRotationOY(), glm::vec3(0, 1, 0));
	RenderSimpleMesh(meshes["worm_body"], shaders["BasicShader"], modelMatrix, mapTextures["worm_body"]);
	RenderSimpleMesh(meshes["worm_eyes"], shaders["BasicShader"], modelMatrix, mapTextures["worm_eyes"]);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 2.5f, 0));
	modelMatrix = glm::rotate(modelMatrix, camera_p1->GetCameraRotationOX() - RADIANS(15), glm::vec3(1, 0, 0));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -2.5f, 0));

	RenderSimpleMesh(meshes["worm_hands"], shaders["BasicShader"], modelMatrix, mapTextures["worm_hands"]);
	RenderSimpleMesh(meshes["bazooka"], shaders["BasicShader"], modelMatrix, mapTextures["bazooka"]);

	modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, player2_pos);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.005f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f, 0.01f, 0.01f));
	modelMatrix = glm::rotate(modelMatrix, camera_p2->GetCameraRotationOY(), glm::vec3(0, 1, 0));
	RenderSimpleMesh(meshes["worm_body"], shaders["BasicShader"], modelMatrix, mapTextures["worm_body"]);
	RenderSimpleMesh(meshes["worm_eyes"], shaders["BasicShader"], modelMatrix, mapTextures["worm_eyes"]);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 2.5f, 0));
	modelMatrix = glm::rotate(modelMatrix, camera_p2->GetCameraRotationOX() - RADIANS(15), glm::vec3(1, 0, 0));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -2.5f, 0));
	RenderSimpleMesh(meshes["worm_hands"], shaders["BasicShader"], modelMatrix, mapTextures["worm_hands"]);
	RenderSimpleMesh(meshes["bazooka"], shaders["BasicShader"], modelMatrix, mapTextures["bazooka"]);

	ProjectileManager(deltaTimeSeconds);
}

void Worms::FrameEnd()
{
	/*if (turn == 0)
		DrawCoordinatSystem(camera_global->GetViewMatrix(), projectionMatrix);
	if (turn == 1)
		DrawCoordinatSystem(camera_p1->GetViewMatrix(), projectionMatrix);
	if (turn == 2)
		DrawCoordinatSystem(camera_p2->GetViewMatrix(), projectionMatrix);*/
}

unsigned int Worms::loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void Worms::RenderCubemap(const glm::mat4& modelMatrix)
{
	Mesh* mesh = meshes["cube"];
	Shader* shader = shaders["SkyboxShader"];
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	glUseProgram(shader->program);

	Laborator::Camera* camera_ptr = camera_global;
	if (turn == 0)
		camera_ptr = camera_global;
	if (turn == 1)
		camera_ptr = camera_p1;
	if (turn == 2)
		camera_ptr = camera_p2;
	if (turn == 3)
		camera_ptr = camera_projectile;

	glm::mat4 viewMatrix = camera_ptr->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glUniform1i(glGetUniformLocation(shader->program, "skybox"), 1);

	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

void Worms::RenderMap(const glm::mat4& modelMatrix, unsigned char* heightMap, Texture2D* texture2)
{
	Mesh* mesh = meshes["square"];

	Shader* shader = shaders["MapShader"];
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	Laborator::Camera* camera_ptr = camera_global;
	if (turn == 0)
		camera_ptr = camera_global;
	if (turn  == 1)
		camera_ptr = camera_p1;
	if (turn  == 2)
		camera_ptr = camera_p2;
	if (turn  == 3)
		camera_ptr = camera_projectile;

	glm::vec3 light_dir = glm::vec3(0, -1, 0);
	int loc_light_dir = glGetUniformLocation(shader->program, "light_direction");
	glUniform3fv(loc_light_dir, 1, glm::value_ptr(light_dir));

	int loc_light_pos1 = glGetUniformLocation(shader->program, "light_position1");
	glUniform3fv(loc_light_pos1, 1, glm::value_ptr(player1_pos + glm::vec3(0, 1, 0)));

	int loc_light_pos2 = glGetUniformLocation(shader->program, "light_position2");
	glUniform3fv(loc_light_pos2, 1, glm::value_ptr(player2_pos + glm::vec3(0,1,0)));

	int loc_eye_pos = glGetUniformLocation(shader->program, "eye_position");
	glUniform3fv(loc_eye_pos, 1, glm::value_ptr(camera_ptr->position));

	int loc_light_angle = glGetUniformLocation(shader->program, "light_angle");
	glUniform1f(loc_light_angle, RADIANS(20));
	

	glm::mat4 viewMatrix = camera_ptr->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	int loc_map_size = glGetUniformLocation(shader->program, "heightMapSize");
	glUniform2fv(loc_map_size, 0, glm::value_ptr(glm::vec2(width, height)));
	
	GLuint texture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)heightPixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	glUniform1i(glGetUniformLocation(shader->program, "heightMap"), 0);

	if (texture2)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2->GetTextureID());	
		glUniform1i(glGetUniformLocation(shader->program, "texture"), 1);
	}

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

void Worms::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture1)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix;
	if (turn == 0)
		viewMatrix = camera_global->GetViewMatrix();
	if (turn == 1)
		viewMatrix = camera_p1->GetViewMatrix();
	if (turn == 2)
		viewMatrix = camera_p2->GetViewMatrix();
	if (turn == 3)
		viewMatrix = camera_projectile->GetViewMatrix();

	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


	if (texture1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1->GetTextureID());
		glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);
	}

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

float Worms::GetHeightOnMapPoint(float x, float z) {
	return heightPixels[(int)floor((z /4) * height) * width + (int)floor((x / 4) * width)] / 255.0f / 2;
}


void Worms::ProjectileManager(float deltaTimeSeconds) {
	if (projectile) {
		projectile->position = projectile->position + projectile->direction * deltaTimeSeconds;
		//projectile->direction = glm::mat3(glm::rotate(glm::mat4(1), -(float)M_PI_4 * deltaTimeSeconds, projectile->right)) * projectile->direction;
		projectile->direction = projectile->direction - glm::vec3(0, 1, 0) * 0.5f * deltaTimeSeconds;
		camera_projectile->Set(projectile->position + glm::vec3(0.3f), projectile->position, glm::vec3(0, 1, 0));
	

		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, projectile->position);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f));
		RenderSimpleMesh(meshes["sphere"], shaders["BasicShader"], modelMatrix);
		if (projectile->position.x < 0 || projectile->position.z < 0 || projectile->position.x > 4.0f || projectile->position.z > 4.0f)
			projectile = NULL;

		wait_timer = 0;
	}


	if (projectile && GetHeightOnMapPoint(projectile->position.x, projectile->position.z) >= projectile->position.y) {
		int impact_point_z = floor(projectile->position.z/4 * height);
		int impact_point_x = floor(projectile->position.x/4 * width);
		int high_value = heightPixels[impact_point_z * width + impact_point_x];

		for (int z = -explosion_radius; z <= explosion_radius; z++) {
			float limit = sin(acos((float)z / explosion_radius)) * explosion_radius;
			for (int x = -floor(limit); x <= floor(limit); x++) {
				int real_x = impact_point_x + x;
				int real_z = impact_point_z + z;
				real_x = min(max(real_x, 0), width);
				real_z = min(max(real_z, 0), height);

				int explosion_height = 10*sqrt(explosion_radius * explosion_radius - x * x - z * z);

				heightPixels[real_z * width + real_x] = max(min((int) heightPixels[real_z * width + real_x], high_value - explosion_height),0);
				
			}
		}
		
		wait_timer = 0;

		player1_pos.y = GetHeightOnMapPoint(player1_pos.x, player1_pos.z);
		camera_p1->Set(player1_pos + glm::vec3(0.1f), player1_pos, glm::vec3(0, 1, 0));
		camera_p1->distanceToTarget = glm::distance(player1_pos + glm::vec3(0.1f), player1_pos);
		camera_p1->TranslateUpword(0.1f);
		
		player2_pos.y = GetHeightOnMapPoint(player2_pos.x, player2_pos.z);
		camera_p2->Set(player2_pos + glm::vec3(0.1f), player2_pos, glm::vec3(0, 1, 0));
		camera_p2->distanceToTarget = glm::distance(player1_pos + glm::vec3(0.1f), player1_pos);
		camera_p2->TranslateUpword(0.1f);
		
		projectile = NULL;
	}

	if (projectile == NULL && turn == 3) {
		if (wait_timer > 1)
			turn = next_turn;
		else
			wait_timer += deltaTimeSeconds;
	}
}

void Worms::OnInputUpdate(float deltaTime, int mods)
{
	float speed = 2;

	Laborator::Camera* camera_ptr = camera_global;

	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) && turn == 0)
	{
		float cameraSpeed = 2.0f;

		if (window->KeyHold(GLFW_KEY_W))
			camera_ptr->MoveForward(cameraSpeed * deltaTime);
		if (window->KeyHold(GLFW_KEY_A))
			camera_ptr->TranslateRight(-cameraSpeed * deltaTime);
		if (window->KeyHold(GLFW_KEY_S))
			camera_ptr->MoveForward(-cameraSpeed * deltaTime);
		if (window->KeyHold(GLFW_KEY_D))
			camera_ptr->TranslateRight(cameraSpeed * deltaTime);
		if (window->KeyHold(GLFW_KEY_Q))
			camera_ptr->TranslateUpword(-cameraSpeed * deltaTime);
		if (window->KeyHold(GLFW_KEY_E))
			camera_ptr->TranslateUpword(cameraSpeed * deltaTime);
	}
}

void Worms::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE)
	{
		switch (turn)
		{
		case 1:
			turn = 2;
			break;
		case 2:
			turn = 1;
			break;
		default:
			break;
		}
	}
	if (key == GLFW_KEY_C) {
		if (turn == 0)
			turn = 1;
		else
			turn = 0;
	}
	// add key press event
}

void Worms::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Worms::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{

	float sensivityOX = 0.001f;
	float sensivityOY = 0.001f;

	if (turn == 1 || turn == 2) {
		glfwSetInputMode(window->GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		Laborator::Camera* camera_ptr = camera_p1;

		if (turn == 2) {
			camera_ptr = camera_p2;
		}

		camera_ptr->RotateThirdPerson_OX(-sensivityOX * deltaY);
		camera_ptr->RotateThirdPerson_OY(-sensivityOY * deltaX);
		
	}


	if (turn == 0) {
		if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
			glfwSetInputMode(window->GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
		{
			Laborator::Camera* camera_ptr = camera_global;

			camera_ptr->RotateFirstPerson_OX(-sensivityOX * deltaY);
			camera_ptr->RotateFirstPerson_OY(-sensivityOY * deltaX);
		}
	}
}

void Worms::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
	if (button == 1 && turn != 0 && turn != 3) {
		projectile = new ProjectileInfo();
		if (turn == 1) {
			projectile->position = player1_pos;
			projectile->position.y += 0.03f;
			projectile->position -= camera_p1->right * 0.01f;
			projectile->right = camera_p1->right;
			projectile->direction = camera_p1->forward;
		}
		else if (turn == 2) {
			projectile->position = player2_pos;
			projectile->position.y += 0.03f;
			projectile->position -= camera_p2->right * 0.01f;
			projectile->right = camera_p2->right;
			projectile->direction = camera_p2->forward;
		}

		if (turn == 1)
			next_turn = 2;
		else if (turn == 2)
			next_turn = 1;

		turn = 3;
	}
}

void Worms::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Worms::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Worms::OnWindowResize(int width, int height)
{
}
