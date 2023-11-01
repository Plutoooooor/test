#include "Angel.h"
#include "TriMesh.h"

#include <vector>
#include <string>
#include<iostream>
using namespace std;

const int X_AXIS = 0;
const int Y_AXIS = 1;
const int Z_AXIS = 2;

const int TRANSFORM_SCALE = 0;
const int TRANSFORM_ROTATE = 1;
const int TRANSFORM_TRANSLATE = 2;

const double DELTA_DELTA = 0.3;		// Delta�ı仯��
const double DEFAULT_DELTA = 0.5;	// Ĭ�ϵ�Deltaֵ

double scaleDelta = DEFAULT_DELTA;
double rotateDelta = DEFAULT_DELTA;
double translateDelta = DEFAULT_DELTA;

glm::vec3 scaleTheta(1.0, 1.0, 1.0);		// ���ſ��Ʊ���
glm::vec3 rotateTheta(0.0, 0.0, 0.0);    // ��ת���Ʊ���
glm::vec3 translateTheta(0.0, 0.0, 0.0);	// ƽ�ƿ��Ʊ���

int currentTransform = TRANSFORM_ROTATE;	// ���õ�ǰ�任

bool is_dragging = false;
bool is_lastPosRecord = false;

int mainWindow;

double lastX;
double lastY;

glm::mat4 arcball(1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);
glm::mat4 arcball_pre(1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);

struct openGLObject
{
	// �����������
	GLuint vao;
	// ���㻺�����
	GLuint vbo;

	// ��ɫ������
	GLuint program;
	// ��ɫ���ļ�
	std::string vshader;
	std::string fshader;
	// ��ɫ������
	GLuint pLocation;
	GLuint cLocation;
	GLuint matrixLocation;
	GLuint darkLocation;
};


openGLObject cube_object;

TriMesh* cube = new TriMesh();


void bindObjectAndData(TriMesh* mesh, openGLObject& object, const std::string& vshader, const std::string& fshader) {

	// ���������������
	glGenVertexArrays(1, &object.vao);  	// ����1�������������
	glBindVertexArray(object.vao);  	// �󶨶����������


	// ��������ʼ�����㻺�����
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->getPoints().size() * sizeof(glm::vec3) + mesh->getColors().size() * sizeof(glm::vec3),
		NULL,
		GL_STATIC_DRAW);

	// @TODO: �޸���TriMesh.cpp�Ĵ���ɺ��ٴ�����ע�ͣ��������ᱨ��
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3), mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);

	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

	// �Ӷ�����ɫ���г�ʼ�������λ��
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// �Ӷ�����ɫ���г�ʼ���������ɫ
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));

	// ��þ���洢λ��
	object.matrixLocation = glGetUniformLocation(object.program, "matrix");
	object.darkLocation = glGetUniformLocation(object.program, "dark");
}


void init()
{
	std::string vshader, fshader;
	// ��ȡ��ɫ����ʹ��
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";

	cube->generateCube();
	bindObjectAndData(cube, cube_object, vshader, fshader);

	// ��ɫ����
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
	// ������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(cube_object.program);

	glBindVertexArray(cube_object.vao);


	// ��ʼ���任����
	glm::mat4 m(1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);

	// @TODO: �ڴ˴��޸ĺ������������յı任����
	// ���ú����������ֱ仯�ı仯�����ۼӵõ��仯����
	// ע�����ֱ仯�ۼӵ�˳��
	glm::mat4 trans = glm::mat4(1.0f);
	m = arcball * arcball_pre * m;

	// ��ָ��λ��matrixLocation�д���任����m
	glUniformMatrix4fv(cube_object.matrixLocation, 1, GL_FALSE, glm::value_ptr(m));

	// �����������еĸ���������
	glDrawArrays(GL_TRIANGLES, 0, cube->getPoints().size());
}

// ͨ��Deltaֵ����Theta
void updateTheta(int axis, int sign) {
	switch (currentTransform) {
		// ���ݱ任���ͣ����ӻ����ĳ�ֱ任�ı仯��
	case TRANSFORM_SCALE:
		scaleTheta[axis] += sign * scaleDelta;
		break;
	case TRANSFORM_ROTATE:
		rotateTheta[axis] += sign * rotateDelta;
		break;
	case TRANSFORM_TRANSLATE:
		translateTheta[axis] += sign * translateDelta;
		break;
	}
}

void dark_time() {

	GLfloat current_time = (float)glfwGetTime();
	glUniform1f(cube_object.darkLocation, (sin(current_time) / 2.0f) + 0.5f);
}

// ��ԭTheta��Delta
void resetTheta()
{
	scaleTheta = glm::vec3(1.0, 1.0, 1.0);
	rotateTheta = glm::vec3(0.0, 0.0, 0.0);
	translateTheta = glm::vec3(0.0, 0.0, 0.0);
	scaleDelta = DEFAULT_DELTA;
	rotateDelta = DEFAULT_DELTA;
	translateDelta = DEFAULT_DELTA;
}

// ���±仯Deltaֵ
void updateDelta(int sign)
{
	switch (currentTransform) {
		// ���ݱ仯�������ӻ����ÿһ�α仯�ĵ�λ�仯��
	case TRANSFORM_SCALE:
		scaleDelta += sign * DELTA_DELTA;
		break;
	case TRANSFORM_ROTATE:
		rotateDelta += sign * DELTA_DELTA;
		break;
	case TRANSFORM_TRANSLATE:
		translateDelta += sign * DELTA_DELTA;
		break;
	}
}


glm::vec3 GetArcballVector(double x, double y)
{
	// ���������ϵĵ�
	double xNorm = (2.0 * x - 600) / 600;
	double yNorm = (600 - 2.0 * y) / 600;
	double length = glm::length(glm::vec2(xNorm, yNorm));

	if (length > 1.0)
	{
		length = 1.0;
	}

	double z = sqrt(1.0 - length * length);

	return glm::vec3(xNorm, yNorm, z);
}

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest)
{
	// ������ת��Ԫ������һ��������ת����һ������
	start = glm::normalize(start);
	dest = glm::normalize(dest);

	float cosTheta = glm::dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f)
	{
		// ���������180����ת
		rotationAxis = glm::cross(glm::vec3(0.0, 0.0, 1.0), start);
		if (glm::length(rotationAxis) < 0.01)
		{
			rotationAxis = glm::cross(glm::vec3(1.0, 0.0, 0.0), start);
		}
		rotationAxis = glm::normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = glm::cross(start, dest);

	float s = sqrt((1.0 + cosTheta) * 2.0);
	float invs = 1.0 / s;

	return glm::quat(s * 0.5, rotationAxis.x * invs, rotationAxis.y * invs, rotationAxis.z * invs);
}



void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	static glm::vec3 lastPos; // ��һ�����λ��

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		if (!is_dragging)
		{
			is_dragging = true;
			lastX = xpos;
			lastY = ypos;
		}
		else
		{
			// ��������ƶ�������
			double deltaX = xpos - lastX;
			double deltaY = ypos - lastY;

			// ����ArcBall��ת
			glm::vec3 v1 = GetArcballVector(lastX, lastY);
			glm::vec3 v2 = GetArcballVector(xpos, ypos);
			glm::quat rotation = RotationBetweenVectors(v2, v1);
			arcball = glm::mat4_cast(rotation) * arcball;

			lastX = xpos;
			lastY = ypos;
		}
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		is_dragging = false;
	}
}





void printHelp() {
	printf("%s\n\n", "3D Transfomations");
	printf("Keyboard options:\n");
	printf("1: Transform Scale\n");
	printf("2: Transform Rotate\n");
	printf("3: Transform Translate\n");
	printf("q: Increase x\n");
	printf("a: Decrease x\n");
	printf("w: Increase y\n");
	printf("s: Decrease y\n");
	printf("e: Increase z\n");
	printf("d: Decrease z\n");
	printf("r: Increase delta of currently selected transform\n");
	printf("f: Decrease delta of currently selected transform\n");
	printf("t: Reset all transformations and deltas\n");
}

void cleanData() {
	cube->cleanData();

	// �ͷ��ڴ�
	delete cube;
	cube = NULL;

	// ɾ���󶨵Ķ���
	glDeleteVertexArrays(1, &cube_object.vao);

	glDeleteBuffers(1, &cube_object.vbo);
	glDeleteProgram(cube_object.program);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv)
{
	// ��ʼ��GLFW�⣬������Ӧ�ó�����õĵ�һ��GLFW����
	glfwInit();

	// ����GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// ���ô�������
	GLFWwindow* window = glfwCreateWindow(600, 600, "3D Transfomations", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_pos_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// �����κ�OpenGL�ĺ���֮ǰ��ʼ��GLAD
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	init();
	// ���������Ϣ
	printHelp();
	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		display();
		//dark_time();
		// ������ɫ���� �Լ� �����û�д���ʲô�¼�������������롢����ƶ��ȣ�
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glutIdleFunc(idle);
	//glutMainLoop();

	cleanData();


	return 0;
}

// ÿ�����ڸı��С��GLFW�������������������Ӧ�Ĳ������㴦��
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}