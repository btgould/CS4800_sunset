#pragma once

#include <cstring>
#include <glm/glm.hpp>
#include <sys/types.h>

#ifndef GRID_COUNT
	#define GRID_COUNT 50
#endif // !GRID_COUNT

enum CellType { CELL_TYPE_EMPTY = 0, CELL_TYPE_FLUID = 1, CELL_TYPE_SOLID = 2 };

struct GridData {
	u_int32_t gridCount;
	alignas(4) CellType cells[GRID_COUNT][GRID_COUNT];
};

class CellGrid {
  public:
	CellGrid();
	~CellGrid() = default;

	void step();
	void write(CellType type, uint32_t row, uint32_t col);

	GridData getGridData() {
		GridData data;
		data.gridCount = GRID_COUNT;
		memcpy(data.cells, m_grid, sizeof(CellType) * GRID_COUNT * GRID_COUNT);

		size_t test = offsetof(GridData, cells);

		return data;
	}

  private:
	CellType m_grid[GRID_COUNT][GRID_COUNT];
};
