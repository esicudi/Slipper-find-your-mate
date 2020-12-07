#pragma once
#ifndef __SLIPPER_H__
#define __SLIPPER_H__

struct slipper_t
{
	vec3	center = vec3(0);
	vec2	v = vec2(0, 0);
	float	velocity_g = 3.0f;
	float	velocity_s = 10.0f;
	float	theta = 0.0f;
	float	psi = 0.0f;
	bool	b_player = false;
	vec4	color;
	mat4	model_matrix;

	// public functions
	void	update(float t);
};

inline std::vector<slipper_t> create_slippers()
{
	std::vector<slipper_t> slippers;
	slipper_t s;

	vec4 color;
	color.r = 117/256.0f;
	color.g = 142/256.0f;
	color.b = 182/256.0f;
	color.a = 1.0f;

	// Player
	s = { vec3(-19 * 40.0f - 20.0f+7,-20 * 40.0f - 20.0f,3), vec2(0,0) };
	s.b_player = true;
	s.color = color;
	slippers.emplace_back(s);

	// Goal
	s = { vec3(19 * 40.0f + 20.0f -7,20 * 40.0f + 20.0f,3), vec2(0,0) };
	s.b_player = false;
	s.color = color;
	slippers.emplace_back(s);

	// Billon-5

	return slippers;
}

inline void slipper_t::update(float t)
{
	float c = cos(theta), s = sin(theta);
	float ct = cos(psi), st = sin(psi);

	center.x += v.x * t;
	center.y += v.y * t;

	mat4 rotation_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	rotation_matrix = rotation_matrix *
	mat4({
		1, 0, 0, 0,
		0,ct,-st,0,
		0,st,ct, 0,
		0, 0, 0, 1
	});

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix;
}

#endif