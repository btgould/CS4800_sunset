#pragma once

#include <cstring>
#include <glm/glm.hpp>
#include <sys/types.h>

#ifndef GRID_COUNT
	#define GRID_COUNT 50
#endif // !GRID_COUNT

enum CellType {
	CELL_TYPE_EMPTY = 1,
	CELL_TYPE_SOLID = 2,
	CELL_TYPE_FLUID_LEFT = 4,
	CELL_TYPE_FLUID_RIGHT = 8,
	CELL_TYPE_SAND = 16,
	CELL_TYPE_FUNGI = 32,
};

struct GridData {
	u_int32_t gridCount;
	alignas(4) CellType cells[GRID_COUNT][GRID_COUNT];
};

class CellGrid {
  public:
	CellGrid();
	~CellGrid() = default;

	void step(double dt);
	void write(CellType type, uint32_t row, uint32_t col);
	void clear();

	GridData getGridData() {
		GridData data;
		data.gridCount = GRID_COUNT;
		memcpy(data.cells, m_grid, sizeof(CellType) * GRID_COUNT * GRID_COUNT);

		size_t test = offsetof(GridData, cells);

		return data;
	}

  private:
	bool isEmpty(uint32_t row, uint32_t col);
	bool isReplacable(uint32_t row, uint32_t col);

  private:
	CellType m_grid[GRID_COUNT][GRID_COUNT];
	bool m_updated[GRID_COUNT][GRID_COUNT];

	float m_updateTime = 6.0f; // default to updating 10x / sec
	float m_accumulatedTime = 0.0f;
};