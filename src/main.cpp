#include "cgmath.h"
#include "cgut.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//************************************
// global constants
static const char*	window_name = "Sleep For Grade";
static const char*	vert_shader_path = "../bin/shaders/sleepForGrade.vert";
static const char*	frag_shader_path = "../bin/shaders/sleepForGrade.frag";

//************************************
// common structures
struct camera
{
	vec3	eye = vec3(30, -100, 10);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 0, 1);
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f;
	float	aspect_ratio;
	float	dNear = 1.0f;
	float	dFar = 100.0f;
	mat4	projection_matrix;
};

struct light_t
{
	vec4	position = vec4(1.0f, -1.0f, 1.0f, 0.0f);
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 1000.0f;
};

//************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = ivec2(800, 450);

//************************************
// OpenGL objects
GLuint	program = 0;
GLuint	vertex_array = 0;

//************************************
// global variables
int		frame = 0;
int		mode = 0;
float	t0 = 0.0f;
float	t1 = 0.0f;
bool	b_pause = false;

//************************************
// scene objects
camera		cam;
light_t		light;
material_t	material;

//************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>		vertices;

//************************************
void update()
{
	// update projection matrix
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);

	// update global simulation parameter
	t1 = float(glfwGetTime()) - t0;
	t0 = float(glfwGetTime());
	if (b_pause)	t1 = 0.0f;

	// update uniform variables in vertex/fragment shaders
	GLint	uloc;
	uloc = glGetUniformLocation(program, "view_matrix");		if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);

	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// notify GL that we use our own program
	glUseProgram(program);

	// bind vertex array object
	glBindVertexArray(vertex_array);
}

void reshape(GLFWwindow* window, int width, int height)
{
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

// text

// create vertices

// create vertex array (index buffer)

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{

	}
	else if (action == GLFW_RELEASE)
	{

	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{

}

void motion(GLFWwindow* window, double x, double y)
{

}

bool user_init()
{

	// init GL states
	glLineWidth(1.0f);
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// define the position of four corner vertices

	// create vertex buffer

	t0 = float(glfwGetTime());

	return true;
}

void user_finalize()
{
}

int main(int argc, char* argv[])
{
	// create window and initialize OpenGL extensions
	if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
	if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }

	//initializations and validations of GLSL program
	if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }

	// register event callbacks
	glfwSetWindowSizeCallback(window, reshape);
	glfwSetKeyCallback(window, keyboard);
	glfwSetMouseButtonCallback(window, mouse);
	glfwSetCursorPosCallback(window, motion);

	// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++)
	{
		glfwPollEvents();
		update();
		render();
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}