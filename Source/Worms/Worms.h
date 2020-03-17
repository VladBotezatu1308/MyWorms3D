#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include "PerlinNoise/PerlinNoise.h"
#include "LabCamera.h"
#include <time.h>
#include <stb/stb_image.h>
#include <vector>

typedef struct {
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 right;
} ProjectileInfo;

class Worms : public SimpleScene
{
public:
	Worms();
	~Worms();

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	
	unsigned int loadCubemap(std::vector<std::string> faces);
	void RenderCubemap(const glm::mat4& modelMatrix);

	void RenderMap(const glm::mat4& modelMatrix, unsigned char* heightMap, Texture2D* texture2 = NULL);
	void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture1 = NULL);

	float GetHeightOnMapPoint(float x, float z);
	void ProjectileManager(float deltaTimeSeconds);

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;

	unsigned int cubemapTexture;
	std::unordered_map<std::string, Texture2D*> mapTextures;
	int height, width, channels;
	unsigned char* heightPixels;
	int state = 0;

	glm::vec3 player1_pos;
	glm::vec3 player2_pos;
	int turn = 1;
	int next_turn = 2;

	Laborator::Camera* camera_p1;
	Laborator::Camera* camera_p2;
	Laborator::Camera* camera_projectile;
	Laborator::Camera* camera_global;

	glm::mat4 projectionMatrix;

	ProjectileInfo* projectile = NULL;
	int explosion_radius = 10;
	float wait_timer;

	GLenum polygonMode = GL_FILL;
};
