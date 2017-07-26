
#include "Precompiled.h"
#include "RenderObject.h"
#include "framework/Application.h"
#include "framework/Debug.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"
#include "graphics/ShaderManager.h"
#include "graphics/ShaderProgram.h"
#include "graphics/VertexArrayObject.h"
#include "graphics/TriangleMesh.h"
#include "graphics/MeshLoader.h"
#include "graphics/Light.h"
#include "graphics/Color.h"
#include "math/Math.h"
//#include "math\Vector3.h"

std::vector<float> RecordTime;
std::vector<int> RecordDirction;

static void Initialize(Application *application, void *udata);
static void Update(Application *application, void *udata);
static void Cleanup(Application *application, void *udata);

int main(int argc, char *argv[])
{
	static int const WindowWidth = 800;
	static int const WindowHeight = 600;

	Application application("CS300 Assignment 3",
		WindowWidth, WindowHeight);
	application.Initialize(argc, argv);
	application.Run(Initialize, Update, Cleanup);

	// application has closed, return from main
	return 0;
}

static f32 oldTime = 0.f;

using namespace Graphics;
using namespace Math;

struct
{
	Color ambient;
	Color diffuse;
	Color specular;
	Color emissive;
	f32 shininess;
} Material;

struct
{
	f32 Near;
	f32 Far;
	Color Color;
} Fog;

enum NormalDebugMode {
	NONE,
	VERTEX_NORMALS,
	FACE_NORMALS,
	BOTH,
	DEBUG_MODE_COUNT
};

enum class LightingScenario {
	SAME_COLOR,
	DIFFERENT_COLOR,
	RANDOM,
	COUNT
};

enum class NormalMapDebugMode {
	NONE,
	TANGENT,
	BITANGENT,
	NORMALMAP,
	COUNT
};

static LightingScenario scenario = LightingScenario::SAME_COLOR;
static std::unique_ptr<ShaderManager> shaderManager;
static std::unique_ptr<TextureManager> textureManager;

static RenderObject renderObj;
static RenderObject plane;

static float timer = 0;

static Vector3 cameraEye;
static Vector3 cameraTarget;
static Vector3 cameraUp;
static Vector3 cameraMovement;
static Vector3 cameraTargetMovement;
static Vector3 RecordStartPos;
static bool ScreenShot = false;
static bool bCameraRecord= false;
static bool bPlayRecord = false;
static float fov;
static float zNear;
static float zFar;
static bool rotateCamera = false;
static ShaderType shaderType;
static float cameraRotateAngleRad = 0.f;
static float cameraRotateRadius = 10.f;

static int debugMode;
static NormalMapDebugMode normalMapDebugMode;

static Color globalAmbient;
static Vector3 lightAttenuationCoef;
static bool bEnableDiffuseTexture;
static bool bEnableSpecularTexture;
static bool bEnableNormalMapping;
static TextureProjectorFunction textureMappingType = TextureProjectorFunction::CYLINDRICAL;
static bool rotateLights = true;

static std::string modelFile = "cube.obj";
//static std::string textureFileDiffuse = "metal_roof_diff_512x512.tga";
//static std::string textureFileSpecular = "metal_roof_spec_512x512.tga";
static std::string textureFileSpecular = "brick.tga";
static std::string textureFileDiffuse = "metal_roof_diff_512x512.tga";
static int activeLightCount = 1;
static const int maxLightCount = 8;
static Light lights[maxLightCount];	
// static lights in the scene
// TODO(student): create a second light, perhaps something generic to help
// prepare for assignment 2 (2 lights supported for assignment 1,
// 8 for assignment 2)
// helper for createSphere(); not documented well because it is not essential
// for CS300 assignment 1
static inline Vector3 CreateSpherePoint(f32 theta, f32 phi, Vector3 &norm)
{
	f32 sinPhi = sinf(phi);
	norm = Vector3(cosf(theta) * sinPhi, cosf(phi), sinf(theta) * sinPhi);
	// fix radius = 1 and center at (0, 0, 2)
	return norm + Vector3(0, 0, 2.f);
}

// Note: this code is not at all necessary for assignment 1. It is provided to
// demonstrate that the lighting works and to show a more sophisticated example
// of creating a triangle mesh. Feel free to re-use this code in future
// assignments. You can also just test lighting right off of loaded meshes, but
// it might be easier to try and get the entire application working before
// worrying about meshes from the disk.
static TriangleMesh *createSphere()
{
	// Generate base wrapping for a sphere; does not include polar caps
	TriangleMesh *mesh = new TriangleMesh;
	f32 deltaPhi = 0.22439947525641380274733167023425f; // hardcode 12 stacks
	f32 deltaTheta = 0.52359877559829887307710723054658f; // hardcode 12 slices
	f32 phi = deltaPhi;
	// remark: could easily optimize this by not duplicating points
	u32 vertexCount = 0;
	for (u8 y = 0; y < 12; ++y) // stacks represents # of actual strips
	{
		f32 theta = 0.f;
		f32 nextPhi = phi + deltaPhi;
		for (u8 x = 0; x < 12; ++x)
		{
			f32 nextTheta = theta + deltaTheta;
			Vector3 normA, normB, normC, normD;
			Vector3 a = CreateSpherePoint(theta, phi, normA);
			Vector3 b = CreateSpherePoint(nextTheta, phi, normB);
			Vector3 c = CreateSpherePoint(nextTheta, nextPhi, normC);
			Vector3 d = CreateSpherePoint(theta, nextPhi, normD);
			mesh->AddVertex(a.x, a.y, a.z);
			mesh->AddVertex(b.x, b.y, b.z);
			mesh->AddVertex(c.x, c.y, c.z);
			mesh->AddVertex(d.x, d.y, d.z);
			u32 idxA = vertexCount++, idxB = vertexCount++;
			u32 idxC = vertexCount++, idxD = vertexCount++;
			mesh->GetVertex(idxA).normal = normA.Normalized();
			mesh->GetVertex(idxB).normal = normB.Normalized();
			mesh->GetVertex(idxC).normal = normC.Normalized();
			mesh->GetVertex(idxD).normal = normD.Normalized();
			mesh->AddTriangle(idxA, idxB, idxC);
			mesh->AddTriangle(idxA, idxC, idxD);
			theta = nextTheta;
		}
		phi = nextPhi;
	}

	mesh->Preprocess();

	return mesh;
}

static TriangleMesh *createXZPlane()
{
	TriangleMesh *mesh = new TriangleMesh;

	// construct vertices for a single triangle
	mesh->AddVertex(-1, 0, -1);
	mesh->AddVertex(1, 0, -1);
	mesh->AddVertex(1, 0, 1);
	mesh->AddVertex(-1, 0, 1);

	// stitch these vertices together in CCW order to create a triangle
	mesh->AddTriangle(0, 2, 1);
	mesh->AddTriangle(0, 3, 2);

	mesh->Preprocess();

	// should also call mesh->Preprocess() here, but that will mess up our sample
	// until translation is implemented (will cause the object to be centered
	// about the origin, thereby causing nothing to draw); make sure to call this
	// as a last step in the mesh loading process
	return mesh;
}

// This is the core example used by createSampleMesh to demonstrate how to
// build a basic triangle mesh.
static TriangleMesh *createTriangle()
{
	TriangleMesh *mesh = new TriangleMesh;

	// construct vertices for a single triangle
	mesh->AddVertex(0.f, 0.5f, 1.f);
	mesh->AddVertex(-0.5f, -0.5f, 1.f);
	mesh->AddVertex(0.5f, -0.5f, 1.f);
	// stitch these vertices together in CCW order to create a triangle
	mesh->AddTriangle(0, 1, 2);

	mesh->Preprocess();

	// should also call mesh->Preprocess() here, but that will mess up our sample
	// until translation is implemented (will cause the object to be centered
	// about the origin, thereby causing nothing to draw); make sure to call this
	// as a last step in the mesh loading process
	return mesh;
}

//////////////////////////////////////////////////////////////////////////
// TODO(student): use the MeshLoader to load a file based on the filename
// provided by the user; note that this should initially be 'cube.obj' if no
// name was provided

// instead, this function is going to show how to construct a mesh from
// scratch using the TriangleMesh data structure; this logic is exactly what
// you should use when loading an OBJ file from the disk
//////////////////////////////////////////////////////////////////////////

static void loadRenderObjMesh()
{
	if (std::shared_ptr<TriangleMesh> model = MeshLoader::LoadMesh(modelFile, textureMappingType))
	{
		renderObj.mesh = model;
		std::shared_ptr<ShaderProgram> const &program =
			shaderManager->GetShader(shaderType);
		renderObj.mesh->Build(program);

		plane.mesh = std::shared_ptr<TriangleMesh>(createXZPlane());
		plane.scale = Vector3(10, 1, 10);
		plane.pos = renderObj.pos;
		plane.pos.y -= 5;
		plane.mesh->Build(shaderManager->GetShader(ShaderType::DEBUG));
	}
}

static void loadShaders()
{
	shaderManager->RegisterShader(ShaderType::PHONG_LIGHT,
		"phonglight.vert", "phonglight.frag")->Build();
	shaderManager->RegisterShader(ShaderType::PHONG_SHADE,
		"phongshade.vert", "phongshade.frag")->Build();
	shaderManager->RegisterShader(ShaderType::BLINN_SHADE,
		"blinnshade.vert", "blinnshade.frag")->Build();
	shaderManager->RegisterShader(ShaderType::DEBUG,
		"debug.vert", "debug.frag")->Build();
	shaderManager->RegisterShader(ShaderType::BLINN_NORMAL,
		"blinnshade_normalmap.vert", "blinnshade_normalmap.frag")->Build();
	shaderManager->RegisterShader(ShaderType::PHONG_NORMAL,
		"phongshade_normalmap.vert", "phongshade_normalmap.frag")->Build();
	shaderManager->RegisterShader(ShaderType::PHONGLIGHT_NORMAL,
		"phonglight_normalmap.vert", "phonglight_normalmap.frag")->Build();
}

static void UpdateLighting(float time)
{
	if (!rotateLights)
		return;

	const float speed = Math::cPi * 0.5f;
	Vector3 center = renderObj.pos;
	Vector3 boundMin = renderObj.mesh->GetBoundMin();
	Vector3 boundMax = renderObj.mesh->GetBoundMax();
	float z = (boundMax.z - boundMin.z);
	float x = (boundMax.x - boundMin.x);
	float d = sqrtf(z*z + x*x) * 3.f;
	for (int i = 0; i < activeLightCount; ++i)
	{
		lights[i].angleRad += speed * time;
		if (lights[i].angleRad >= Math::cTwoPi)
			lights[i].angleRad -= Math::cTwoPi;
		f32 x = cosf(lights[i].angleRad);
		f32 y = sinf(lights[i].angleRad);
		lights[i].position.x = x * d;
		lights[i].position.z = y * d;
		lights[i].position.y = boundMax.y;
		lights[i].position += Vector4(center.x, center.y, center.z, 0);
		lights[i].direction = Vector4(center.x, center.y, center.z, 1) - lights[i].position;
		lights[i].direction.Normalize();
	}
}

static void ResetLightingPosition()
{
	if (activeLightCount <= 0)
		return;

	Vector3 center = renderObj.pos;
	Vector3 boundMin = renderObj.mesh->GetBoundMin();
	Vector3 boundMax = renderObj.mesh->GetBoundMax();
	float z = (boundMax.z - boundMin.z);
	float x = (boundMax.x - boundMin.x);
	float d = sqrtf(z*z + x*x) * 1.5f;
	float angleInterval = Math::cTwoPi / activeLightCount;
	float curAngle = angleInterval;
	for (int i = 0; i < activeLightCount; ++i)
	{
		lights[i].angleRad = curAngle;
		f32 x = cosf(curAngle);
		f32 y = sinf(curAngle);
		lights[i].distFromCenter = d;
		lights[i].position.x = x * d;
		lights[i].position.z = y * d;
		lights[i].position.y = boundMax.y;
		lights[i].position += Vector4(center.x, center.y, center.z, 0);
		lights[i].direction = Vector4(center.x, center.y, center.z, 1) - lights[i].position;
		lights[i].direction.Normalize();
		curAngle += angleInterval;
	}
}

static void ResetLightingScenario()
{
	static Color colors[8] = {
		Color::Red,
		Color::Green,
		Color::Blue,
		Color::Yellow,
		Color::Cyan,
		Color::Magenta,
		Color::White,
		Color::Gray
	};

	switch (scenario)
	{
	case LightingScenario::SAME_COLOR:
	{
		for (int i = 1; i < 8; ++i)
		{
			lights[i].type = lights[0].type;
			lights[i].ambient = lights[0].ambient;
			lights[i].specular = lights[0].specular;
			lights[i].diffuse = lights[0].diffuse;
			lights[i].spotlightFalloff = lights[0].spotlightFalloff;
			lights[i].spotlightInnerAngleRad = lights[0].spotlightInnerAngleRad;
			lights[i].spotlightOuterAngleRad = lights[0].spotlightOuterAngleRad;
		}
		break;
	}
	case LightingScenario::DIFFERENT_COLOR:
	{
		for (int i = 0; i < 8; ++i)
		{
			lights[i].type = LightType(i % ((int)LightType::MAX));
			lights[i].specular = Color::White;
			lights[i].diffuse = colors[i % 8];
		}
		break;
	}
	case LightingScenario::RANDOM:
	{
		for (int i = 0; i < 8; ++i)
		{
			lights[i].type = LightType(rand() % ((int)LightType::MAX));
			lights[i].specular = colors[rand() % 8];
			lights[i].diffuse = Color::White;
		}
		break;
	}
	}
}

static void EnableLight(Application* application, ShaderProgram* program, int idx)
{
	Matrix4 modelview = Matrix4::LookAt(cameraEye, cameraTarget, cameraUp) *
		Matrix4::Translate(lights[idx].position.x, lights[idx].position.y, lights[idx].position.z);

	std::stringstream sstream;
	sstream.str(std::string());
	sstream << "Lights[" << idx << "]";
	program->SetUniform(sstream.str() + ".type", (u32)lights[idx].type);
	program->SetUniform(sstream.str() + ".position", Transform(modelview, lights[idx].position));
	program->SetUniform(sstream.str() + ".direction", Transform(modelview, lights[idx].direction));
	program->SetUniform(sstream.str() + ".ambient", lights[idx].ambient);
	program->SetUniform(sstream.str() + ".diffuse", lights[idx].diffuse);
	program->SetUniform(sstream.str() + ".specular", lights[idx].specular);
	program->SetUniform(sstream.str() + ".spotlight_innerCos", Math::Cos(lights[idx].spotlightInnerAngleRad));
	program->SetUniform(sstream.str() + ".spotlight_outerCos", Math::Cos(lights[idx].spotlightOuterAngleRad));
	program->SetUniform(sstream.str() + ".spotlight_falloff", lights[idx].spotlightFalloff);
}

static void enableSampleLights(Application* application, std::shared_ptr<ShaderProgram> program)
{
	program->SetUniform("globalAmbient", globalAmbient);
	program->SetUniform("vLightAttCoef", lightAttenuationCoef);
	program->SetUniform("Fog.near", Fog.Near);
	program->SetUniform("Fog.far", Fog.Far);
	program->SetUniform("Fog.color", Fog.Color);
	program->SetUniform("LightCount", (u32)activeLightCount);

	for (int i = 0; i < activeLightCount; ++i)
		EnableLight(application, program.get(), i);
}

void loadTextures()
{
	textureManager->RegisterTexture(TextureType::DIFFUSE, textureFileDiffuse)->Build();
	textureManager->RegisterTexture(TextureType::SPECULAR, textureFileSpecular)->Build();
	textureManager->RegisterNormalMapTexture(TextureType::NORMAL, textureFileSpecular)->Build();
}

void loadLights()
{
	std::shared_ptr<ShaderProgram> const &program =
		shaderManager->GetShader(ShaderType::DEBUG);
	for (int i = 0; i < maxLightCount; ++i)
	{
		lights[i].type = LightType::Point;
		lights[i].direction = Vector4(0, 0, 1, 0);
		lights[i].ambient = Color::Black;
		lights[i].diffuse = Color(0.8f, 0.8f, 0.8f, 1);
		lights[i].specular = Color::White;
		lights[i].spotlightInnerAngleRad = Math::DegToRad(15);
		lights[i].spotlightOuterAngleRad = Math::DegToRad(30);
		lights[i].spotlightFalloff = 1;
		lights[i].position = Vector4(0, 0, 1, 1);
		lights[i].mesh = std::shared_ptr<TriangleMesh>(createSphere());
		lights[i].mesh->Build(program);
		lights[i].radius = 0.25f;
	}
}


enum flag
{
	UniformBlur = 1,
	DepthOfField = 2,
	Bloom = 4,
	AdditiveNoise = 8,
	RGB2HSV = 16,
	ScratchedFilm = 32,
	ToneChange = 64,
	HueChange = 128,
	HalfTone = 256,
	WaterColor = 512,
};
void TakeScreenShot(Application *application)
{
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	unsigned char *image = (unsigned char*)malloc(sizeof(unsigned char)*application->GetWindowWidth()*application->GetWindowHeight() * 3);
	FILE *file = fopen("capture.bmp", "wb");

	if (image != NULL)
	{
		if (file != NULL)
		{
			glReadPixels(0, 0, application->GetWindowWidth(), application->GetWindowHeight(), GL_BGR_EXT, GL_UNSIGNED_BYTE, image);

			memset(&bf, 0, sizeof(bf));
			memset(&bi, 0, sizeof(bi));

			bf.bfType = 'MB';
			bf.bfSize = sizeof(bf) + sizeof(bi) + application->GetWindowWidth()*application->GetWindowHeight() * 3;
			bf.bfOffBits = sizeof(bf) + sizeof(bi);
			bi.biSize = sizeof(bi);
			bi.biWidth = application->GetWindowWidth();
			bi.biHeight = application->GetWindowHeight();
			bi.biPlanes = 1;
			bi.biBitCount = 24;
			bi.biSizeImage = application->GetWindowWidth()*application->GetWindowHeight() * 3;

			fwrite(&bf, sizeof(bf), 1, file);
			fwrite(&bi, sizeof(bi), 1, file);
			fwrite(image, sizeof(unsigned char), application->GetWindowHeight()*application->GetWindowWidth() * 3, file);

			fclose(file);
		}
		free(image);
	}
}
void PlayCameraRecord(f32 time)
{
	static float RecordTimer = 0;
	switch (RecordDirction.at(0))
	{
	case 0:
		cameraMovement = Vector3(0.f, 0.f, 0.5f);
		break;
	case 1:
		cameraMovement = Vector3(0.5f, 0.f, 0.0f);
		break;
	case 2:
		cameraMovement = Vector3(0.f, 0.f, -0.5f);
		break;
	case 3:
		cameraMovement = Vector3(-0.5f, 0.f, 0.0f);
		break;
	}
	cameraEye += cameraMovement*time;
	RecordTimer += time;
	if (RecordTimer >= RecordTime.at(0))
	{
		if(!RecordTime.empty());
		RecordTime.front() = std::move(RecordTime.back());
		RecordTime.pop_back();
		
		if (!RecordDirction.empty());
		RecordDirction.front() = std::move(RecordDirction.back());
		RecordDirction.pop_back();
		RecordTimer = 0;
	}
	if (RecordTime.size() == 0)
	{
		bPlayRecord = false;
		cameraMovement = Vector3(0.f, 0.f, 0.0f);
	}
		
}

int shaderflag = 0;
void getKeyInput(unsigned char key, int xmouse, int ymouse)
{
	if (key == 'q')
	{
		(shaderflag & flag::UniformBlur) != flag::UniformBlur ?
			shaderflag += flag::UniformBlur : shaderflag -= flag::UniformBlur;
	}

	if (key == 'w')
	{
		(shaderflag & flag::DepthOfField) != flag::DepthOfField ?
			shaderflag += flag::DepthOfField : shaderflag -= flag::DepthOfField;
	}

	if (key == 'e')
	{
		(shaderflag & flag::Bloom) != flag::Bloom ?
			shaderflag += flag::Bloom : shaderflag -= flag::Bloom;
	}

	if (key == 'r')
	{
		(shaderflag & flag::AdditiveNoise) != flag::AdditiveNoise ?
			shaderflag += flag::AdditiveNoise : shaderflag -= flag::AdditiveNoise;
	}

	if (key == 'a')
	{
		(shaderflag & flag::RGB2HSV) != flag::RGB2HSV ?
			shaderflag += flag::RGB2HSV : shaderflag -= flag::RGB2HSV;
	}

	if (key == 's')
	{
		(shaderflag & flag::ScratchedFilm) != flag::ScratchedFilm ?
			shaderflag += flag::ScratchedFilm : shaderflag -= flag::ScratchedFilm;
	}

	if (key == 'd')
	{
		(shaderflag & flag::ToneChange) != flag::ToneChange ?
			shaderflag += flag::ToneChange : shaderflag -= flag::ToneChange;
	}

	if (key == 'f')
	{
		(shaderflag & flag::HueChange) != flag::HueChange ?
			shaderflag += flag::HueChange : shaderflag -= flag::HueChange;
	}

	if (key == 'z')
	{
		(shaderflag & flag::HalfTone) != flag::HalfTone ?
			shaderflag += flag::HalfTone : shaderflag -= flag::HalfTone;
	}
	if (key == 'i')
	{//left
		if (bCameraRecord)
		{
			RecordTime.push_back(timer);
			RecordDirction.push_back(0);
		}
		timer = 0;
		cameraMovement = Vector3(0.f, 0.f, 0.5f);
	}
	if (key == 'j')
	{//forword
		if (bCameraRecord)
		{
			RecordTime.push_back(timer);
			RecordDirction.push_back(1);
		}
		timer = 0;
		cameraMovement = Vector3(0.5f, 0.f, 0.0f);
		//cameraTargetMovement = Vector3(0.5f, 0.f, 0.0f);
	}
	if (key == 'k')
	{//backword
		if (bCameraRecord)
		{
			RecordTime.push_back(timer);
			RecordDirction.push_back(2);
		}
		timer = 0;
		cameraMovement = Vector3(0.f, 0.f, -0.5f);
	}
	if (key == 'l')
	{//right
		if (bCameraRecord)
		{

			RecordTime.push_back(timer);
			RecordDirction.push_back(3);
		}
		timer = 0;
		cameraMovement = Vector3(-0.5f, 0.f, 0.0f);
		//cameraTargetMovement = Vector3(-0.5f, 0.f, 0.0f);
	}
	if (key == ' ')
	{// stop camera movement with stop record if it plays
		cameraMovement = Vector3(0.f, 0.f, 0.0f);
//		cameraTargetMovement = Vector3(0.f, 0.f, 0.0f);
		bCameraRecord = false;
		timer = 0;
	}
	if (key == 'p')
	{// take screenshot
		ScreenShot = true;
	}
	if (key == '1')
	{// record Camera movement
		timer = 0;
		bCameraRecord = true;
		RecordStartPos = cameraEye;
	}
	if (key == '2')
	{// play Carmera record
		bCameraRecord = false;
		timer = 0;
	//	ScreenShot = true;
	//	();
		bPlayRecord = true;
		cameraEye = RecordStartPos;
	}


	glutPostRedisplay(); //request display() call ASAP
}


void Initialize(Application *application, void *udata)
{
	// create the class used to manage shader programs
	shaderManager = std::unique_ptr<ShaderManager>(new ShaderManager);
	textureManager = std::unique_ptr<TextureManager>(new TextureManager);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f); // set background color to medium gray
	glEnable(GL_DEPTH_TEST); // enable the depth buffer and depth testing
	
	glutKeyboardFunc(getKeyInput);

	renderObj.pos = Vector3(0.f, 0.f, 0.f);
	renderObj.eulerRotate = Vector3(0.f, 0.f, 0.f);
	debugMode = NONE;

	globalAmbient = Color(0.2f, 0.2f, 0.2f, 1.f);
	lightAttenuationCoef = Vector3(1.f, 0.1f, 0);

	cameraEye = Vector3(0.f, 0.f, -2.f);
	cameraTarget = Vector3(0.f, 0.f, 0.f);
	cameraUp = Vector3(0.f, 1.f, 0.f);
	cameraMovement = Vector3(0,0,0);
	cameraTargetMovement = Vector3(0, 0, 0);
	fov = 1.f;
	zNear = 0.1f;
	zFar = 100.f;

	Fog.Near = (zFar - zNear) * 0.5f;
	Fog.Far = zFar;
	Fog.Color = Color::White;

	Material.ambient = Color(0.25f, 0.25f, 0.25f);
	Material.diffuse = Color(0.25f, 0.25f, 0.25f);
	Material.specular = Color(0.25f, 0.25f, 0.25f);
	Material.emissive = Color::Black;
	Material.shininess = 50;
	shaderType = ShaderType::BLINN_NORMAL;

	bEnableDiffuseTexture = false;
	bEnableSpecularTexture = false;
	bEnableNormalMapping = true;
	textureMappingType = TextureProjectorFunction::CYLINDRICAL;

	activeLightCount = 1;

	loadShaders();
	loadRenderObjMesh();
	loadTextures();
	loadLights();

	ResetLightingPosition();
	ResetLightingScenario();

	oldTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
}

void RenderUI()
{
	ImGui::Begin("Debug Window");

	char fileNameBuffer[140] = { '\0' };
	std::strcat(fileNameBuffer, modelFile.c_str());
	if (ImGui::InputText("Model", fileNameBuffer, sizeof(fileNameBuffer)))
	{
		// text was changed; copy back over to C++ string
		modelFile = fileNameBuffer;
	}

	if (ImGui::Button("Load Model"))
	{
		loadRenderObjMesh();
	}

	if (ImGui::Button("Reload Shaders"))
	{
		shaderManager->ClearShaders();
		loadShaders();
	}

	std::vector<char const *> debugmodestrings = {
		"None", "Vertex Normals", "Face Normals", "Both"
	};
	ImGui::Combo("debug mode", &debugMode, debugmodestrings.data(), DEBUG_MODE_COUNT);

	std::vector<char const *> normalmapDebugmodeStrings = {
		"None", "Tangent", "Bitangent", "Normalmap Normal"
	};
	int normalMapDebugModeInt = (int)normalMapDebugMode;
	ImGui::Combo("Normal map debug mode", &normalMapDebugModeInt, normalmapDebugmodeStrings.data(), (int)NormalMapDebugMode::COUNT);
	normalMapDebugMode = (NormalMapDebugMode)normalMapDebugModeInt;

	ImGui::Separator();
	// model options

	ImGui::PushID("ModelOptions");

	ImGui::InputFloat3("Model Position", renderObj.pos.ToFloats());
	ImGui::SliderAngle("Rotation X", &renderObj.eulerRotate.x);
	ImGui::SliderAngle("Rotation Y", &renderObj.eulerRotate.y);
	ImGui::SliderAngle("Rotation Z", &renderObj.eulerRotate.z);

	if (ImGui::CollapsingHeader("Material", 0, true, true))
	{
		std::vector<char const *> shaderTypeStrings = {
			"Graud",
			"Phong",
			"Blinn",
			"Debug",
			"Blinn with Normalmap",
			"Phong with Normalmap",
			"Graud with Normalmap"
		};
		int shadertype = (int)shaderType;
		ImGui::Combo("Light Type", &shadertype, shaderTypeStrings.data(), (int)ShaderType::COUNT);
		shaderType = (ShaderType)shadertype;
		ImGui::ColorEdit3("Ambient", Material.ambient.ToFloats());

		ImGui::Checkbox("Use Diffuse Texture", &bEnableDiffuseTexture);
		if (!bEnableDiffuseTexture)
			ImGui::ColorEdit3("Diffuse", Material.diffuse.ToFloats());

		ImGui::Checkbox("Use Texture Shininess", &bEnableSpecularTexture);
		//if (!bEnableSpecularTexture)
		ImGui::ColorEdit3("Specular", Material.specular.ToFloats());

		ImGui::Checkbox("Use Normal Mapping", &bEnableNormalMapping);

		int projType = (int)textureMappingType;
		std::vector<char const *> projectorTypeStrings = {
			"CYLINDRICAL",
			"SPHERICAL",
			"CUBIC"
		};
		ImGui::Combo("Texture Mapping Type", &projType, projectorTypeStrings.data(), (int)TextureProjectorFunction::COUNT);
		if (textureMappingType != TextureProjectorFunction(projType))
		{
			textureMappingType = TextureProjectorFunction(projType);
			loadRenderObjMesh();
		}

		ImGui::SliderFloat("Shininess", &Material.shininess, 0, 100, "%.2f");
		ImGui::ColorEdit3("Emissive", Material.emissive.ToFloats());
	}
	ImGui::PopID();

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Camera"))
	{
		if (ImGui::Checkbox("Rotate", &rotateCamera))
		{
			if (rotateCamera)
			{
				cameraEye = renderObj.pos - Vector3(0,0,-3);
			}
		}
		ImGui::InputFloat3("Eye", cameraEye.ToFloats());
		cameraTarget = renderObj.pos;
		if (cameraEye == cameraTarget)
			cameraTarget = cameraEye + Vector3(0, 0, 1);
	}

	if (ImGui::CollapsingHeader("Global Lighting"))
	{
	ImGui::ColorEdit3("Global Ambient", globalAmbient.ToFloats());
	ImGui::InputFloat3("Light Attenuation Coefficient", lightAttenuationCoef.ToFloats());
	ImGui::InputFloat("Fog Near", &Fog.Near);
	ImGui::InputFloat("Fog Far", &Fog.Far);
	ImGui::ColorEdit3("Fog Color", Fog.Color.ToFloats());
	}

	if (ImGui::CollapsingHeader("Lighting", 0, true, true))
	{
		if (ImGui::SliderInt("Light Count", &activeLightCount, 0, maxLightCount))
		{
			ResetLightingPosition();
		}

		std::vector<char const *> scenarioTypeStrings = {
			"Same color/type",
			"Different color/type",
			"Random",
		};
		ImGui::Checkbox("Rotate", &rotateLights);
		int s = (int)scenario;
		if (ImGui::Combo("Light Scenario", &s, scenarioTypeStrings.data(), (int)LightingScenario::COUNT))
		{
			scenario = (LightingScenario)s;
			ResetLightingScenario();
		}

		if (ImGui::Button("Reset Scenario"))
			ResetLightingScenario();
	}

	for (int idx = 0; idx < activeLightCount; ++idx)
	{
		std::stringstream sstream;
		sstream << "Light " << idx;
		ImGui::PushID(sstream.str().c_str());
		if (ImGui::CollapsingHeader(sstream.str().c_str()))
		{
			std::vector<char const *> lightTypeStrings = {
				"Directional",
				"Spot",
				"Point"
			};
			int lighttype = (int)lights[idx].type;
			ImGui::Combo("Light Type", &lighttype, lightTypeStrings.data(), (int)LightType::MAX);
			lights[idx].type = (LightType)lighttype;

			ImGui::InputFloat3("Position", lights[idx].position.ToFloats());
			switch (lights[idx].type)
			{
			case LightType::Directional:
				ImGui::ColorEdit3("Ambient", lights[idx].ambient.ToFloats());
				ImGui::ColorEdit3("Diffuse", lights[idx].diffuse.ToFloats());
				ImGui::ColorEdit3("Specular", lights[idx].specular.ToFloats());
				break;
			case LightType::Point:
				ImGui::ColorEdit3("Ambient", lights[idx].ambient.ToFloats());
				ImGui::ColorEdit3("Diffuse", lights[idx].diffuse.ToFloats());
				ImGui::ColorEdit3("Specular", lights[idx].specular.ToFloats());
				break;
			case LightType::Spot:
				ImGui::ColorEdit3("Ambient", lights[idx].ambient.ToFloats());
				ImGui::ColorEdit3("Diffuse", lights[idx].diffuse.ToFloats());
				ImGui::ColorEdit3("Specular", lights[idx].specular.ToFloats());
				ImGui::SliderAngle("Spotlight Inner Angle", &lights[idx].spotlightInnerAngleRad, 0, 90);
				ImGui::SliderAngle("Spotlight Outer Angle", &lights[idx].spotlightOuterAngleRad, 0, 90);
				ImGui::InputFloat("Spotlight Fall-off", &lights[idx].spotlightFalloff);
				break;
			}
		}
		ImGui::PopID();
	}

	ImGui::End();
}


void RenderMesh(Application* application)
{
	const Vector3& modelRotate = renderObj.eulerRotate;
	Matrix4 modelview = Matrix4::LookAt(cameraEye, cameraTarget, cameraUp) *
		Matrix4::Translate(renderObj.pos) *
		Matrix4::RotateEuler(modelRotate.z, modelRotate.x, modelRotate.y);
	Matrix4 proj = Matrix4::PerspectiveProjection(fov, application->GetWindowWidth(), application->GetWindowHeight(), zNear, zFar);
	Matrix4 mvp = proj * modelview; // model-view-projection concatenated matrix

	// Render
	if (shaderManager)
	{
		auto &program = shaderManager->GetShader(shaderType);
		program->Bind();

		program->SetUniform("ModelViewMatrix", modelview);
		program->SetUniform("ModelViewProjectionMatrix", mvp);

		program->SetUniform("Material.ambient", Material.ambient);
		program->SetUniform("Material.diffuse", Material.diffuse);
		program->SetUniform("Material.emissive", Material.emissive);
		program->SetUniform("Material.specular", Material.specular);
		program->SetUniform("Material.shininess", Material.shininess);
		program->SetUniform("useDiffuseTexture", (u32)bEnableDiffuseTexture);
		program->SetUniform("useSpecularTexture", (u32)bEnableSpecularTexture);
		program->SetUniform("useNormalMapping", (u32)bEnableNormalMapping);
		program->SetUniform("normalMapDebugMode", (u32)normalMapDebugMode);

		program->SetUniform("shaderflag", (u32)shaderflag);
		GLint m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);
		program->SetUniform("res", Math::Vector3(m_viewport[2], m_viewport[3], 0));
		// program->SetUniform("TIME", (u32)normalMapDebugMode);
		program->SetUniform("myTextureSampler", (u32)0);
		program->SetUniform("eye", cameraEye);

		// GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
				
		textureManager->BindAttach(TextureType::DIFFUSE, program, "diffuseTexture");
		textureManager->BindAttach(TextureType::SPECULAR, program, "specularTexture");
		textureManager->BindAttach(TextureType::NORMAL, program, "normalTexture");

		enableSampleLights(application, program);

		if (renderObj.mesh)
		{
			renderObj.mesh->Render();

			program->SetUniform("LightCount", 0U);
			switch (static_cast<NormalDebugMode>(debugMode))
			{
			case VERTEX_NORMALS:
				renderObj.mesh->RenderVertexNormals();
				renderObj.mesh->RenderVertexTangents();
				renderObj.mesh->RenderVertexBitangents();
				break;
			case FACE_NORMALS:
				renderObj.mesh->RenderFaceNormals();
				break;
			case BOTH:
				renderObj.mesh->RenderVertexNormals();
				renderObj.mesh->RenderVertexTangents();
				renderObj.mesh->RenderVertexBitangents();
				renderObj.mesh->RenderFaceNormals();
				break;
			default:
				break;
			}
			program->SetUniform("LightCount", (u32)activeLightCount);
			// disable lights for debug drawing, leading to black colors for lines
		}

		// render plane
		{
			Matrix4 modelview = Matrix4::LookAt(cameraEye, cameraTarget, cameraUp) *
				Matrix4::Translate(plane.pos) *
				Matrix4::Scale(plane.scale);
			Matrix4 mvp = proj * modelview; // model-view-projection concatenated matrix
			program->SetUniform("ModelViewMatrix", modelview);
			program->SetUniform("ModelViewProjectionMatrix", mvp);
			plane.mesh->Render();
		}

		textureManager->Unbind(TextureType::DIFFUSE);
		textureManager->Unbind(TextureType::SPECULAR);
		textureManager->Unbind(TextureType::NORMAL);
		program->Unbind();
	}
}

void RenderLights(Application* application)
{
	if (shaderManager)
	{
		auto &program = shaderManager->GetShader(ShaderType::DEBUG);
		program->Bind();

		Matrix4 proj = Matrix4::PerspectiveProjection(fov, application->GetWindowWidth(), application->GetWindowHeight(), zNear, zFar);

		for (int i = 0; i < activeLightCount; ++i)
		{
			Matrix4 modelview = Matrix4::LookAt(cameraEye, cameraTarget, cameraUp) *
				Matrix4::Translate(lights[i].position.x, lights[i].position.y, lights[i].position.z) *
				Matrix4::Scale(lights[i].radius, lights[i].radius, lights[i].radius);
			Matrix4 mvp = proj * modelview; // model-view-projection concatenated matrix

			// Render
			program->SetUniform("ModelViewProjectionMatrix", mvp);
			program->SetUniform("Color", lights[i].diffuse);

			lights[i].mesh->Render();
		}

		program->Unbind();
	}
}

void UpdateCamera(float time)
{
//	
//	if (timer < 0.5f)
//	{
	cameraEye += cameraMovement*time;
//	cameraTarget += cameraTargetMovement*time;
	if(bCameraRecord)
		timer += time;
//	}

	if (!rotateCamera)
		return;

	f32 speed = Math::cTwoPi * 0.1f;
	cameraRotateAngleRad += speed * time;
	if (cameraRotateAngleRad > Math::cTwoPi)
		cameraRotateAngleRad -= Math::cTwoPi;

	cameraTarget = renderObj.pos;
	cameraEye.x = renderObj.pos.x + cameraRotateRadius * cosf(cameraRotateAngleRad);
	cameraEye.z = renderObj.pos.z + cameraRotateRadius * sinf(cameraRotateAngleRad);
	cameraEye.y = renderObj.pos.y + 4 * sinf(cameraRotateAngleRad);

}

void Update(Application *application, void *udata)
{
	f32 newTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
	f32 dt = newTime - oldTime;
	oldTime = newTime;

	UpdateCamera(dt);
	UpdateLighting(dt);

	// clear the pixel and depth buffers for this frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderUI();
	
	RenderMesh(application);

	RenderLights(application);

	if (ScreenShot == true)
	{
		TakeScreenShot(application);
		ScreenShot = false;
	}
	if (bPlayRecord)
		PlayCameraRecord(dt);
	// done creating ImGui components
}
void GetCameraMovement(int key)
{

}



void Cleanup(Application *application, void *udata)
{
	// cleanup OpenGL resources and allocated memory
	shaderManager = nullptr; // delete all programs
	// delete all meshes

	renderObj.mesh = nullptr;
	for (int i = 0; i < maxLightCount; ++i)
		lights[i].mesh = nullptr;
}
