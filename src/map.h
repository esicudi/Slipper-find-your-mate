#pragma once
#ifndef __MAP_H__
#define __MAP_H__

struct map_t
{
	vec3	center = vec3(0);
	float	l = 0.0f;
	vec4	color;
	mat4	model_matrix;

	// public functions
	void	update();
};

inline std::vector<map_t> create_map()
{
	std::vector<map_t> map;
	map_t m;

	//starting point
	m.l = 1.0f;
	m.center = vec3(-19 * 40.0f - 20.0f, -20 * 40.0f - 20.0f, 0.0f);
	map.emplace_back(m);

	//goal point
	m.l = 1.0f;
	m.center = vec3(19 * 40.0f + 20.0f, 20 * 40.0f + 20.0f, 0.0f);
	map.emplace_back(m);

	//map
	m.l = 40.0f;
	m.center = vec3(0.0f, 0.0f, 0.0f);
	map.emplace_back(m);

	return map;
}

inline void map_t::update()
{
	mat4 scale_matrix =
	{
		l, 0, 0, 0,
		0, l, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * scale_matrix;
}

#endif