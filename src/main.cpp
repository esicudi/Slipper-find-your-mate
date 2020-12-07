#include "cgmath.h"
#include "cgut.h"
#include "slipper.h"
#include "map.h"
#include "foot.h"
#include <string>
#include "irrKlang\irrKlang.h"
#pragma comment(lib, "irrKlang.lib")

//************************************
// global constants
static const char* window_name = "Slipper, find your mate!";
static const char* vert_shader_path = "../bin/shaders/sleepForGrade.vert";
static const char* frag_shader_path = "../bin/shaders/sleepForGrade.frag";
static const char* fly_mp3_path = "../bin/sounds/fly.mp3";
static const char* end_mp3_path = "../bin/sounds/end.mp3";

//************************************
// common structures
struct camera
{
	vec3	eye = vec3(0, -100, -150);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 0, 1);
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f;
	float	aspect_ratio;
	float	dNear = 1.0f;
	float	dFar = 10000.0f;
	mat4	projection_matrix;
};

struct light_t
{
	vec4	position = vec4(0.0f, 0.3f, 1.0f, 0.0f);
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
ivec2		window_size = ivec2(1152, 864);

//************************************
// OpenGL objects
GLuint	program = 0;
GLuint	vertex_array = 0;
GLuint	map_vertex_array = 0;
GLuint	slipper_vertex_array = 0;
GLuint	foot_vertex_array = 0;

//*******************************************************************
// irrKlang objects
irrklang::ISoundEngine* engine;
irrklang::ISoundSource* fly_mp3_src = nullptr;
irrklang::ISoundSource* end_mp3_src = nullptr;

//************************************
// global variables
int		frame = 0;
int		mode = 5;
float	t0 = 0.0f;
float	t1 = 0.0f;
float	t = 0.0f;
float	t_play = 0.0f;
float	a = 0.0f;
bool	b_start = false;
bool	b_reset = false;
bool	b_fly = false;
bool	b_kick = false;
bool	b_end = false;
bool	b_help = false;
auto	slippers = std::move(create_slippers());
auto	map = std::move(create_map());
auto	foots = std::move(create_foots(mode));
struct { bool up = false, down = false, right = false, left = false; operator bool() const { return up || down || right || left; } } b;

//************************************
// scene objects
camera		cam;
light_t		light;
material_t	material;

//************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>		unit_map_vertices;
std::vector<vertex>		unit_slipper_vertices;
std::vector<vertex>		unit_foot_vertices;

//*******************************************************************
// forward declarations for freetype text
bool init_text();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color, GLfloat dpi_scale = 1.0f);

void kick(foot_t f)
{
	//t += t1;
}

void fly(slipper_t& s)
{
	if (s.center.z <= 3)
	{
		s.center.z = 3;
		s.v = vec2(0.0f);
		b_fly = false;
		s.psi = 0.0f;
		//engine->stopAllSoundsOfSoundSource(fly_mp3_src);
	}
	else
	{
		b.up = false; b.down = false; b.left = false; b.right = false;
		t += t1;
		s.psi = 4.0f * t;
		s.center.z = s.center.z + 10.0f * t - 20.0f * t * t / 2;
	}
}

//************************************
void update()
{
	// update projection matrix
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);

	// update global simulation parameter
	t1 = float(glfwGetTime()) - t0;
	t0 = float(glfwGetTime());
	if (!b_end)	t_play += t1;
	a = abs(sin(float(glfwGetTime()) * 2.5f));

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

	GLint uloc;

	// render texts
	float dpi_scale = cg_get_dpi_scale();
	// starting
	if (!b_start && !b_end)
	{
		render_text("Slipper, find your mate!", 100, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
		std::string str = "mode: < ";
		str.push_back(48 + mode);
		str.push_back(' ');
		str.push_back('>');
		render_text(str, 100, 125, 0.5f, vec4(0.7f, 0.4f, 0.1f, 0.8f), dpi_scale);
		render_text("Press buttun 'S' to start.", 100, 155, 0.6f, vec4(0.5f, 0.7f, 0.7f, a), dpi_scale);
		render_text("Press buttun 'ESC' to end game.", 100, 175, 0.6f, vec4(0.5f, 0.7f, 0.7f, a), dpi_scale);
	}

	// end
	if (b_end)
	{
		std::string str = "Play time: ";
		std::string str1 = std::to_string((int)t_play);
		render_text(str + str1 + "sec", 100, 100, 1.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
		render_text("Press buttun 'S' to go to start buttun.", 100, 155, 0.6f, vec4(0.5f, 0.7f, 0.7f, a), dpi_scale);
		render_text("Press buttun 'ESC' to end game.", 100, 175, 0.6f, vec4(0.5f, 0.7f, 0.7f, a), dpi_scale);
	}

	// play
	else if (b_start) {

		// bind vertex array object
		glBindVertexArray(map_vertex_array);
		for (auto& m : map)
		{
			m.update();

			uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, m.model_matrix);
			uloc = glGetUniformLocation(program, "mode_text");	if (uloc > -1) glUniform1i(uloc, false);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		}

		glBindVertexArray(slipper_vertex_array);
		for (auto& s : slippers)
		{
			s.update(t1);

			if (s.b_player)
			{
				if (b_reset)
				{
					b_fly = false;
					s.center = vec3(-19 * 40.0f - 20.0f + 7, -20 * 40.0f - 20.0f, 3);
					s.theta = 0.0f;
					b_reset = false;
					t_play = 0.0f;
					b_start = false;
				}
				if (b_fly)	fly(s);
				else
				{

					if (b.up || b.down)
					{
						float ct = cos(s.theta), st = sin(s.theta);
						vec2 v = vec2(-st, ct);
						if (b.up)			s.v = v * 40;
						else if (b.down)	s.v = -v * 40;
					}
					else if (!(b.up || b.down))
					{
						if (s.center.z == 3)	s.v = vec2(0, 0);
					}
					if (b.left || b.right)
					{
						if (b.left)		s.theta += t1;
						if (b.right)	s.theta -= t1;
					}
					if (s.center.x < -800.0f || s.center.x>800.0f) { s.center = vec3(-19 * 40.0f - 20.0f + 7, -20 * 40.0f - 20.0f, 3); s.theta = 0.0f; }
					else if (s.center.y < -800.0f)
					{
						if (s.center.x > -760.0f) { s.center = vec3(-19 * 40.0f - 20.0f + 7, -20 * 40.0f - 20.0f, 3); s.theta = 0.0f; }
						else if (s.center.y < -840.0f) { s.center = vec3(-19 * 40.0f - 20.0f + 7, -20 * 40.0f - 20.0f, 3); s.theta = 0.0f; }
					}
					else if (s.center.y > 800.0f)
					{
						if (s.center.x < 760.0f) { s.center = vec3(-19 * 40.0f - 20.0f + 7, -20 * 40.0f - 20.0f, 3); s.theta = 0.0f; }
						else if (s.center.y > 840.0f) { s.center = vec3(-19 * 40.0f - 20.0f + 7, -20 * 40.0f - 20.0f, 3); s.theta = 0.0f; }
					}

					if (s.center.y > 810 && s.center.y < 830)
					{
						if (s.center.x > 780.0f && s.center.x < 800.0f)
						{
							b_end = true;
							b_start = false;
							engine->play2D(end_mp3_src, false, false);
						}
					}
				}

				cam.eye = s.center + vec3(0, -200, 300);
				cam.at = s.center;
				cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);

				// update uniform variables in vertex/fragment shaders
				uloc = glGetUniformLocation(program, "view_matrix");		if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
				uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
			}
			uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, s.model_matrix);
			uloc = glGetUniformLocation(program, "mode_text");	if (uloc > -1) glUniform1i(uloc, true);
			uloc = glGetUniformLocation(program, "color");	if (uloc > -1) glUniform4fv(uloc, 1, s.color);

			glDrawElements(GL_TRIANGLES, 198 + 16 * 3, GL_UNSIGNED_INT, nullptr);
		}
	}

	glBindVertexArray(foot_vertex_array);
	for (auto& f : foots)
	{
		f.update(t1);

		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1)	glUniformMatrix4fv(uloc, 1, GL_TRUE, f.model_matrix);
		uloc = glGetUniformLocation(program, "mode_text");	if (uloc > -1) glUniform1i(uloc, true);
		uloc = glGetUniformLocation(program, "color");	if (uloc > -1) glUniform4fv(uloc, 1, f.color);

		glDrawElements(GL_TRIANGLES, 32 * 3 + 18 * 6, GL_UNSIGNED_INT, nullptr);
	}

	glfwSwapBuffers(window);
}

void collision()
{
	for (auto& s : slippers)
	{
		for (auto& f : foots)
		{
			vec3 center_s = s.center;
			if (center_s.z <= 3)
			{
				vec3 center_f = f.center;
				float d = sqrtf(powf((center_s.x - center_f.x), 2) + powf(center_s.y - center_f.y, 2));
				if (d < 20)
				{
					s.theta = f.theta;
					float ct = cos(s.theta), st = sin(s.theta);
					vec2 v = vec2(-st, ct);
					s.v = 500 * v;
					s.center.x = f.center.x;
					s.center.y = f.center.y;
					if (b_fly)	return;
					b_fly = true;
					kick(f);
					b.up = false; b.down = false; b.left = false; b.right = false;
					t = 0.0f;
					t += t1;
					s.psi = t;
					s.center.z = 3.2f;
					engine->play2D(fly_mp3_src, false, false);
					return;
				}
			}
		}
	}
}

void reshape(GLFWwindow* window, int width, int height)
{
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

// create vertices -map -slipper -foot
std::vector<vertex> create_map_vertices()
{
	std::vector<vertex> v;
	v.push_back({ vec3(20.0f, 20.0f, 0.0f), vec3(0,0,1),vec2(0.0f) });
	v.push_back({ vec3(20.0f, -20.0f, 0.0f), vec3(0,0,1),vec2(0.0f) });
	v.push_back({ vec3(-20.0f, -20.0f, 0.0f), vec3(0,0,1),vec2(0.0f) });
	v.push_back({ vec3(-20.0f, 20.0f, 0.0f), vec3(0,0,1),vec2(0.0f) });

	return v;
}

std::vector<vertex> create_slipper_vertices()
{
	std::vector<vertex> v;
	//cover
	v.push_back({ vec3(-6.001f,8.0f,-2.0f),vec3(-1,0,0),vec2(0.0f) });	//1
	v.push_back({ vec3(-6.001f,0.0f,-2.0f),vec3(-1,0,0),vec2(0.0f) });	//2
	v.push_back({ vec3(-6.001f,8.0f,-1.0f),vec3(-1,0,1),vec2(0.0f) });	//3
	v.push_back({ vec3(-6.001f,0.0f,0.0f),vec3(-4,1,1),vec2(0.0f) });		//4
	v.push_back({ vec3(-5.0f,0.0f,2.0f),vec3(-3,1,2),vec2(0.0f) });		//5
	v.push_back({ vec3(-4.0f,8.0f,1.0f),vec3(-2,1,3),vec2(0.0f) });		//6
	v.push_back({ vec3(-4.0f,0.0f,3.0f),vec3(-2,1,3),vec2(0.0f) });		//7
	v.push_back({ vec3(-2.0f,8.0f,2.0f),vec3(-1,1,4),vec2(0.0f) });		//8
	v.push_back({ vec3(-2.0f,0.0f,4.0f),vec3(-1,1,4),vec2(0.0f) });		//9
	v.push_back({ vec3(2.0f,8.0f,2.0f),vec3(1,1,4),vec2(0.0f) });		//10
	v.push_back({ vec3(2.0f,0.0f,4.0f),vec3(1,1,4),vec2(0.0f) });		//11
	v.push_back({ vec3(4.0f,8.0f,1.0f),vec3(2,1,3),vec2(0.0f) });		//12
	v.push_back({ vec3(4.0f,0.0f,3.0f),vec3(2,1,3),vec2(0.0f) });		//13
	v.push_back({ vec3(5.0f,0.0f,2.0f),vec3(3,1,2),vec2(0.0f) });		//14
	v.push_back({ vec3(6.001f,8.0f,-1.0f),vec3(1,0,1),vec2(0.0f) });		//15
	v.push_back({ vec3(6.001f,0.0f,0.0f),vec3(4,1,1),vec2(0.0f) });		//16
	v.push_back({ vec3(6.001f,8.0f,-2.0f),vec3(1,0,0),vec2(0.0f) });		//17
	v.push_back({ vec3(6.001f,0.0f,-2.0f),vec3(1,0,0),vec2(0.0f) });		//18

	//insole
	v.push_back({ vec3(-6.0f,8.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//1
	v.push_back({ vec3(-5.0f,10.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//2
	v.push_back({ vec3(-4.0f,11.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//3
	v.push_back({ vec3(-2.0f,12.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//4
	v.push_back({ vec3(2.0f,12.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//5
	v.push_back({ vec3(4.0f,11.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//6
	v.push_back({ vec3(5.0f,10.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//7
	v.push_back({ vec3(6.0f,8.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//8
	v.push_back({ vec3(6.0f,-8.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//9
	v.push_back({ vec3(5.0f,-10.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//10
	v.push_back({ vec3(4.0f,-11.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//11
	v.push_back({ vec3(2.0f,-12.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//12
	v.push_back({ vec3(-2.0f,-12.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//13
	v.push_back({ vec3(-4.0f,-11.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//14
	v.push_back({ vec3(-5.0f,-10.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//15
	v.push_back({ vec3(-6.0f,-8.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });	//16
	v.push_back({ vec3(-6.0f,8.0f,-1.0f),vec3(0,0,1),vec2(0.0f) });		//1

	v.push_back({ vec3(-6.0f,8.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//17
	v.push_back({ vec3(-5.0f,10.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//18
	v.push_back({ vec3(-4.0f,11.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//19
	v.push_back({ vec3(-2.0f,12.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//20
	v.push_back({ vec3(2.0f,12.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//21
	v.push_back({ vec3(4.0f,11.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//22
	v.push_back({ vec3(5.0f,10.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//23
	v.push_back({ vec3(6.0f,8.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//24
	v.push_back({ vec3(6.0f,-8.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//25
	v.push_back({ vec3(5.0f,-10.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//26
	v.push_back({ vec3(4.0f,-11.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//27
	v.push_back({ vec3(2.0f,-12.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//28
	v.push_back({ vec3(-2.0f,-12.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//29
	v.push_back({ vec3(-4.0f,-11.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//30
	v.push_back({ vec3(-5.0f,-10.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//31
	v.push_back({ vec3(-6.0f,-8.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });	//32
	v.push_back({ vec3(-6.0f,8.0f,-3.0f),vec3(0,0,-1),vec2(0.0f) });		//17

	//cover-inner
	v.push_back({ vec3(-6.001f,8.0f,-2.0f),vec3(1,0,0),vec2(0.0f) });	//1
	v.push_back({ vec3(-6.001f,0.0f,-2.0f),vec3(1,0,0),vec2(0.0f) });	//2
	v.push_back({ vec3(-6.001f,8.0f,-1.0f),vec3(1,0,-1),vec2(0.0f) });	//3
	v.push_back({ vec3(-6.001f,0.0f,0.0f),vec3(4,-1,-1),vec2(0.0f) });		//4
	v.push_back({ vec3(-5.0f,0.0f,2.0f),vec3(3,-1,-2),vec2(0.0f) });		//5
	v.push_back({ vec3(-4.0f,8.0f,1.0f),vec3(2,-1,-3),vec2(0.0f) });		//6
	v.push_back({ vec3(-4.0f,0.0f,3.0f),vec3(2,-1,-3),vec2(0.0f) });		//7
	v.push_back({ vec3(-2.0f,8.0f,2.0f),vec3(1,-1,-4),vec2(0.0f) });		//8
	v.push_back({ vec3(-2.0f,0.0f,4.0f),vec3(1,-1,-4),vec2(0.0f) });		//9
	v.push_back({ vec3(2.0f,8.0f,2.0f),vec3(-1,-1,-4),vec2(0.0f) });		//10
	v.push_back({ vec3(2.0f,0.0f,4.0f),vec3(-1,-1,-4),vec2(0.0f) });		//11
	v.push_back({ vec3(4.0f,8.0f,1.0f),vec3(-2,-1,-3),vec2(0.0f) });		//12
	v.push_back({ vec3(4.0f,0.0f,3.0f),vec3(-2,-1,-3),vec2(0.0f) });		//13
	v.push_back({ vec3(5.0f,0.0f,2.0f),vec3(-3,-1,-2),vec2(0.0f) });		//14
	v.push_back({ vec3(6.001f,8.0f,-1.0f),vec3(-1,0,-1),vec2(0.0f) });		//15
	v.push_back({ vec3(6.001f,0.0f,0.0f),vec3(-4,-1,-1),vec2(0.0f) });		//16
	v.push_back({ vec3(6.001f,8.0f,-2.0f),vec3(-1,0,0),vec2(0.0f) });		//17
	v.push_back({ vec3(6.001f,0.0f,-2.0f),vec3(-1,0,0),vec2(0.0f) });		//18

	return v;
}

std::vector<vertex> create_foot_vertices()
{
	std::vector<vertex> v;
	v.push_back({ vec3(-5.0f, 3.0f, -32.0f), vec3(-1,1,1),vec2(0.0f) });	//0
	v.push_back({ vec3(-5.0f, -3.0f, -32.0f), vec3(-1,-1,1),vec2(0.0f) });	//1
	v.push_back({ vec3(-5.0f, -3.0f, -40.0f), vec3(-1,-1,-1),vec2(0.0f) });	//2
	v.push_back({ vec3(-5.0f, 17.0f, -40.0f), vec3(-1,1,-1),vec2(0.0f) });	//3
	v.push_back({ vec3(-5.0f, 17.0f, -39.0f), vec3(-1,1,1),vec2(0.0f) });	//4
	v.push_back({ vec3(-5.0f, 16.0f, -39.0f), vec3(-1,1,1),vec2(0.0f) });	//5
	v.push_back({ vec3(-5.0f, 16.0f, -38.0f), vec3(-1,1,1),vec2(0.0f) });	//6
	v.push_back({ vec3(-5.0f, 14.0f, -38.0f), vec3(-1,1,1),vec2(0.0f) });	//7
	v.push_back({ vec3(-5.0f, 14.0f, -37.0f), vec3(-1,1,1),vec2(0.0f) });	//8
	v.push_back({ vec3(-5.0f, 11.0f, -37.0f), vec3(-1,1,1),vec2(0.0f) });	//9
	v.push_back({ vec3(-5.0f, 11.0f, -36.0f), vec3(-1,1,1),vec2(0.0f) });	//10
	v.push_back({ vec3(-5.0f, 7.0f, -36.0f), vec3(-1,1,1),vec2(0.0f) });	//11
	v.push_back({ vec3(-5.0f, 7.0f, -35.0f), vec3(-1,1,1),vec2(0.0f) });	//12
	v.push_back({ vec3(-5.0f, 5.0f, -35.0f), vec3(-1,1,1),vec2(0.0f) });	//13
	v.push_back({ vec3(-5.0f, 5.0f, -34.0f), vec3(-1,1,1),vec2(0.0f) });	//14
	v.push_back({ vec3(-5.0f, 4.0f, -34.0f), vec3(-1,1,1),vec2(0.0f) });	//15
	v.push_back({ vec3(-5.0f, 4.0f, -33.0f), vec3(-1,1,1),vec2(0.0f) });	//16
	v.push_back({ vec3(-5.0f, 3.0f, -33.0f), vec3(-1,1,1),vec2(0.0f) });	//17
	v.push_back({ vec3(-5.0f, 3.0f, -32.0f), vec3(-1,1,1),vec2(0.0f) });	//18

	v.push_back({ vec3(5.0f, 3.0f, -32.0f), vec3(-1,1,1),vec2(0.0f) });		//0+19
	v.push_back({ vec3(5.0f, -3.0f, -32.0f), vec3(-1,-1,1),vec2(0.0f) });	//1
	v.push_back({ vec3(5.0f, -3.0f, -40.0f), vec3(-1,-1,-1),vec2(0.0f) });	//2
	v.push_back({ vec3(5.0f, 17.0f, -40.0f), vec3(-1,1,-1),vec2(0.0f) });	//3
	v.push_back({ vec3(5.0f, 17.0f, -39.0f), vec3(-1,1,1),vec2(0.0f) });	//4
	v.push_back({ vec3(5.0f, 16.0f, -39.0f), vec3(-1,1,1),vec2(0.0f) });	//5
	v.push_back({ vec3(5.0f, 16.0f, -38.0f), vec3(-1,1,1),vec2(0.0f) });	//6
	v.push_back({ vec3(5.0f, 14.0f, -38.0f), vec3(-1,1,1),vec2(0.0f) });	//7
	v.push_back({ vec3(5.0f, 14.0f, -37.0f), vec3(-1,1,1),vec2(0.0f) });	//8
	v.push_back({ vec3(5.0f, 11.0f, -37.0f), vec3(-1,1,1),vec2(0.0f) });	//9
	v.push_back({ vec3(5.0f, 11.0f, -36.0f), vec3(-1,1,1),vec2(0.0f) });	//10
	v.push_back({ vec3(5.0f, 7.0f, -36.0f), vec3(-1,1,1),vec2(0.0f) });		//11
	v.push_back({ vec3(5.0f, 7.0f, -35.0f), vec3(-1,1,1),vec2(0.0f) });		//12
	v.push_back({ vec3(5.0f, 5.0f, -35.0f), vec3(-1,1,1),vec2(0.0f) });		//13
	v.push_back({ vec3(5.0f, 5.0f, -34.0f), vec3(-1,1,1),vec2(0.0f) });		//14
	v.push_back({ vec3(5.0f, 4.0f, -34.0f), vec3(-1,1,1),vec2(0.0f) });		//15
	v.push_back({ vec3(5.0f, 4.0f, -33.0f), vec3(-1,1,1),vec2(0.0f) });		//16
	v.push_back({ vec3(5.0f, 3.0f, -33.0f), vec3(-1,1,1),vec2(0.0f) });		//17
	v.push_back({ vec3(5.0f, 3.0f, -32.0f), vec3(-1,1,1),vec2(0.0f) });		//18

	return v;
}

// create vertex array (index buffer)
void create_map_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;
	static GLuint index_buffer = 0;

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	//check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	// create buffer
	std::vector<uint> indices;
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(1);

	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(2);
	//..............

	//generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (map_vertex_array) glDeleteVertexArrays(1, &map_vertex_array);
	map_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!map_vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void create_slipper_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;
	static GLuint index_buffer = 0;

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	//check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	// create buffer
	std::vector<uint> indices;
	//cover
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(4);

	indices.push_back(2);
	indices.push_back(4);
	indices.push_back(5);

	indices.push_back(4);
	indices.push_back(6);
	indices.push_back(5);

	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(7);

	indices.push_back(7);
	indices.push_back(6);
	indices.push_back(8);

	indices.push_back(8);
	indices.push_back(9);
	indices.push_back(7);

	indices.push_back(8);
	indices.push_back(10);
	indices.push_back(9);

	indices.push_back(9);
	indices.push_back(10);
	indices.push_back(12);

	indices.push_back(9);
	indices.push_back(12);
	indices.push_back(11);

	indices.push_back(11);
	indices.push_back(12);
	indices.push_back(13);

	indices.push_back(11);
	indices.push_back(13);
	indices.push_back(14);

	indices.push_back(14);
	indices.push_back(13);
	indices.push_back(15);

	indices.push_back(14);
	indices.push_back(15);
	indices.push_back(17);

	indices.push_back(14);
	indices.push_back(17);
	indices.push_back(16);

	//insole
	indices.push_back(22);
	indices.push_back(21);
	indices.push_back(23);

	indices.push_back(21);
	indices.push_back(20);
	indices.push_back(23);

	indices.push_back(23);
	indices.push_back(20);
	indices.push_back(24);

	indices.push_back(20);
	indices.push_back(19);
	indices.push_back(24);

	indices.push_back(24);
	indices.push_back(19);
	indices.push_back(25);

	indices.push_back(19);
	indices.push_back(18);
	indices.push_back(25);

	indices.push_back(18);
	indices.push_back(26);
	indices.push_back(25);

	indices.push_back(18);
	indices.push_back(33);
	indices.push_back(26);

	indices.push_back(33);
	indices.push_back(32);
	indices.push_back(26);

	indices.push_back(26);
	indices.push_back(32);
	indices.push_back(27);

	indices.push_back(32);
	indices.push_back(31);
	indices.push_back(27);

	indices.push_back(27);
	indices.push_back(31);
	indices.push_back(28);

	indices.push_back(28);
	indices.push_back(31);
	indices.push_back(30);

	indices.push_back(28);
	indices.push_back(30);
	indices.push_back(29);


	indices.push_back(17 + 22);
	indices.push_back(17 + 23);
	indices.push_back(17 + 21);

	indices.push_back(17 + 21);
	indices.push_back(17 + 23);
	indices.push_back(17 + 20);

	indices.push_back(17 + 23);
	indices.push_back(17 + 24);
	indices.push_back(17 + 20);

	indices.push_back(17 + 20);
	indices.push_back(17 + 24);
	indices.push_back(17 + 19);

	indices.push_back(17 + 24);
	indices.push_back(17 + 25);
	indices.push_back(17 + 19);

	indices.push_back(17 + 19);
	indices.push_back(17 + 25);
	indices.push_back(17 + 18);

	indices.push_back(17 + 18);
	indices.push_back(17 + 25);
	indices.push_back(17 + 26);

	indices.push_back(17 + 18);
	indices.push_back(17 + 26);
	indices.push_back(17 + 33);

	indices.push_back(17 + 33);
	indices.push_back(17 + 26);
	indices.push_back(17 + 32);

	indices.push_back(17 + 26);
	indices.push_back(17 + 27);
	indices.push_back(17 + 32);

	indices.push_back(17 + 32);
	indices.push_back(17 + 27);
	indices.push_back(17 + 31);

	indices.push_back(17 + 27);
	indices.push_back(17 + 28);
	indices.push_back(17 + 31);

	indices.push_back(17 + 28);
	indices.push_back(17 + 30);
	indices.push_back(17 + 31);

	indices.push_back(17 + 28);
	indices.push_back(17 + 29);
	indices.push_back(17 + 30);

	for (int i = 18; i < 34; i++)
	{
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 17);
		indices.push_back(i + 17);
		indices.push_back(i + 1);
		indices.push_back(i + 18);
	}

	//cover-inner
	indices.push_back(52 + 0);
	indices.push_back(52 + 2);
	indices.push_back(52 + 1);

	indices.push_back(52 + 1);
	indices.push_back(52 + 2);
	indices.push_back(52 + 3);

	indices.push_back(52 + 2);
	indices.push_back(52 + 4);
	indices.push_back(52 + 3);

	indices.push_back(52 + 2);
	indices.push_back(52 + 5);
	indices.push_back(52 + 4);

	indices.push_back(52 + 4);
	indices.push_back(52 + 5);
	indices.push_back(52 + 6);

	indices.push_back(52 + 5);
	indices.push_back(52 + 7);
	indices.push_back(52 + 6);

	indices.push_back(52 + 7);
	indices.push_back(52 + 8);
	indices.push_back(52 + 6);

	indices.push_back(52 + 8);
	indices.push_back(52 + 7);
	indices.push_back(52 + 9);

	indices.push_back(52 + 8);
	indices.push_back(52 + 9);
	indices.push_back(52 + 10);

	indices.push_back(52 + 9);
	indices.push_back(52 + 12);
	indices.push_back(52 + 10);

	indices.push_back(52 + 9);
	indices.push_back(52 + 11);
	indices.push_back(52 + 12);

	indices.push_back(52 + 11);
	indices.push_back(52 + 13);
	indices.push_back(52 + 12);

	indices.push_back(52 + 11);
	indices.push_back(52 + 14);
	indices.push_back(52 + 13);

	indices.push_back(52 + 14);
	indices.push_back(52 + 15);
	indices.push_back(52 + 13);

	indices.push_back(52 + 14);
	indices.push_back(52 + 17);
	indices.push_back(52 + 15);

	indices.push_back(52 + 14);
	indices.push_back(52 + 16);
	indices.push_back(52 + 17);

	//generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (slipper_vertex_array) glDeleteVertexArrays(1, &slipper_vertex_array);
	slipper_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!slipper_vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void create_foot_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;
	static GLuint index_buffer = 0;

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	//check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	// create buffer
	std::vector<uint> indices;

	indices.push_back(0);
	indices.push_back(17);
	indices.push_back(1);

	indices.push_back(17);
	indices.push_back(2);
	indices.push_back(1);

	indices.push_back(15);
	indices.push_back(2);
	indices.push_back(17);

	indices.push_back(13);
	indices.push_back(2);
	indices.push_back(15);

	indices.push_back(11);
	indices.push_back(2);
	indices.push_back(13);

	indices.push_back(9);
	indices.push_back(2);
	indices.push_back(11);

	indices.push_back(7);
	indices.push_back(2);
	indices.push_back(9);

	indices.push_back(5);
	indices.push_back(2);
	indices.push_back(7);

	indices.push_back(3);
	indices.push_back(2);
	indices.push_back(5);

	indices.push_back(4);
	indices.push_back(3);
	indices.push_back(5);

	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(7);

	indices.push_back(8);
	indices.push_back(7);
	indices.push_back(9);

	indices.push_back(10);
	indices.push_back(9);
	indices.push_back(11);

	indices.push_back(12);
	indices.push_back(11);
	indices.push_back(13);

	indices.push_back(14);
	indices.push_back(13);
	indices.push_back(15);

	indices.push_back(16);
	indices.push_back(15);
	indices.push_back(17);

	indices.push_back(19 + 0);
	indices.push_back(19 + 1);
	indices.push_back(19 + 17);

	indices.push_back(19 + 17);
	indices.push_back(19 + 1);
	indices.push_back(19 + 2);

	indices.push_back(19 + 15);
	indices.push_back(19 + 17);
	indices.push_back(19 + 2);

	indices.push_back(19 + 13);
	indices.push_back(19 + 15);
	indices.push_back(19 + 2);

	indices.push_back(19 + 11);
	indices.push_back(19 + 13);
	indices.push_back(19 + 2);

	indices.push_back(19 + 9);
	indices.push_back(19 + 11);
	indices.push_back(19 + 2);

	indices.push_back(19 + 7);
	indices.push_back(19 + 9);
	indices.push_back(19 + 2);

	indices.push_back(19 + 5);
	indices.push_back(19 + 7);
	indices.push_back(19 + 2);

	indices.push_back(19 + 3);
	indices.push_back(19 + 5);
	indices.push_back(19 + 2);

	indices.push_back(19 + 4);
	indices.push_back(19 + 5);
	indices.push_back(19 + 3);

	indices.push_back(19 + 6);
	indices.push_back(19 + 7);
	indices.push_back(19 + 5);

	indices.push_back(19 + 8);
	indices.push_back(19 + 9);
	indices.push_back(19 + 7);

	indices.push_back(19 + 10);
	indices.push_back(19 + 11);
	indices.push_back(19 + 9);

	indices.push_back(19 + 12);
	indices.push_back(19 + 13);
	indices.push_back(19 + 11);

	indices.push_back(19 + 14);
	indices.push_back(19 + 15);
	indices.push_back(19 + 13);

	indices.push_back(19 + 16);
	indices.push_back(19 + 17);
	indices.push_back(19 + 15);

	for (int i = 0; i < 18; i++)
	{
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 20);
		indices.push_back(i);
		indices.push_back(i + 20);
		indices.push_back(i + 19);
	}

	//generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (foot_vertex_array) glDeleteVertexArrays(1, &foot_vertex_array);
	foot_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!foot_vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_UP)		b.up = true;
		if (key == GLFW_KEY_DOWN)	b.down = true;
		if (key == GLFW_KEY_LEFT)
		{
			b.left = true;
			if (!b_start) { if (mode > 1) mode--; b.left = false; }
		}
		if (key == GLFW_KEY_RIGHT)
		{
			b.right = true;
			if (!b_start) { if (mode < 9) mode++; b.right = false; }
		}
		else if (key == GLFW_KEY_S)
		{
			if (b_end) { b_end = false; }
			else if (!b_start)
			{
				b_start = true; t_play = 0.0f; foots = std::move(create_foots(mode)); slippers = std::move(create_slippers());
			}
		}
		else if (key == GLFW_KEY_R)	b_reset = true;
		else if (key == GLFW_KEY_ESCAPE)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_F1)	b_help = true;
	}
	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_UP)		b.up = false;
		if (key == GLFW_KEY_DOWN)	b.down = false;
		if (key == GLFW_KEY_LEFT)	b.left = false;
		if (key == GLFW_KEY_RIGHT)	b.right = false;
		else if (key == GLFW_KEY_F1)	b_help = true;
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
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// define the position of four corner vertices
	unit_map_vertices = std::move(create_map_vertices());
	unit_slipper_vertices = std::move(create_slipper_vertices());
	unit_foot_vertices = std::move(create_foot_vertices());

	// create vertex buffer
	create_map_buffer(unit_map_vertices);
	create_slipper_buffer(unit_slipper_vertices);
	create_foot_buffer(unit_foot_vertices);

	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;

	//add sound source from the sound file
	fly_mp3_src = engine->addSoundSourceFromFile(fly_mp3_path);
	end_mp3_src = engine->addSoundSourceFromFile(end_mp3_path);

	//set default volume
	fly_mp3_src->setDefaultVolume(0.5f);
	end_mp3_src->setDefaultVolume(1.0f);

	// setup freetype
	if (!init_text()) return false;

	t0 = float(glfwGetTime());

	return true;
}

void user_finalize()
{
	engine->drop();
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
		collision();
		update();
		render();
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}