#ifndef GAME_STRUCTS_H
#define GAME_STRUCTS_H

#include <cstdint>
#include <iostream>
#include <vector>


struct MazeConfig
{
	static const uint32_t MIN_WIDTH = 3;
	static const uint32_t MIN_HEIGHT = 3;
	static const uint32_t MIN_LEVELS = 1;
	static const uint32_t MAX_WIDTH = 19;
	static const uint32_t MAX_HEIGHT = 11;
	static const uint32_t MAX_LEVELS = 10;

	uint32_t width;
	uint32_t height;
	uint32_t levels;

	MazeConfig() :
		width(0), height(0), levels(0)
	{}
		
	MazeConfig(uint32_t width_, uint32_t height_, uint32_t levels_) :
		width(width_), height(height_), levels(levels_)
	{}

	void print() const
	{
		std::cout << "Maze configuration: [" << "Width=" << width << ", Height=" << height << ", Levels=" << levels << "]" << std::endl;
	}

	uint32_t getTotalRooms() const { return width * height * levels; }
};

typedef std::vector<MazeConfig> maze_config_vec;


struct Vertex3DEx
{
	uint32_t x, y, z, extra;

	Vertex3DEx() :
		x(0), y(0), z(0), extra(0)
	{}

	Vertex3DEx(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _extra = 0) :
		x(_x), y(_y), z(_z), extra(_extra)
	{}

	bool operator==(const Vertex3DEx & other)
	{
		return ( (x == other.x) && (y == other.y) && (z == other.z) );
	}

	bool operator!=(const Vertex3DEx & other) { return !(*this == other); }

	void print() const
	{
		std::cout << "Vertex 3D: [" << "X=" << x << ", Y=" << y << ", Z=" << z << ", Ex=" << extra << "]" << std::endl;
	}
};

typedef std::vector<Vertex3DEx> vertex3d_vec;

#endif // GAME_STRUCTS_H
