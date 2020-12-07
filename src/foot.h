#pragma once
#ifndef __FOOT_H__
#define __FOOT_H__

struct foot_t
{
	vec3	center = vec3(0);
	float	theta = 0.0f;
	float	psi = 0.0f;
	vec4	color;
	mat4	model_matrix;

	// public functions
	void	update(float t);
};

inline float randf(float m, float M)
{
	float r = rand() / float(RAND_MAX - 1);
	return m + (M - m) * r;
}

inline std::vector<foot_t> create_foots(int n)
{
	std::vector<foot_t> foots;
	foot_t f;

	vec4 color;
	color.r = 241 / 256.0f;
	color.g = 218 / 256.0f;
	color.b = 203 / 256.0f;
	color.a = 1.0f;

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++)
		{
			if (randf(0.0f, 1.0f) < n / 10.0f)
			{
				f.center = vec3(-40.0f * 19 + 80.0f * i+ randf(-36.0f, 36.0f), -40.0f * 19 + 80.0f * j+randf(-36.0f, 36.0f), 43.0f);
				f.color = color;
				f.theta = randf(0, 2.0f * PI);
				foots.emplace_back(f);
			}
		}
	}

	return foots;
}

inline void foot_t::update(float t)
{
	float c = cos(theta), s = sin(theta);
	float ct = cos(psi), st = sin(psi);

	

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