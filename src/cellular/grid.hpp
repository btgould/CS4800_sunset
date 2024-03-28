#pragma once

#include <cstring>
#include <sys/types.h>
#ifndef GRID_COUNT
	#define GRID_COUNT 50
#endif // !GRID_COUNT

enum CellType { CELL_TYPE_EMPTY, CELL_TYPE_SOLID, CELL_TYPE_FLUID };

struct GridData {
	u_int32_t gridCount;
	CellType cells[GRID_COUNT][GRID_COUNT];
};

class CellGrid {
  public:
	CellGrid();
	~CellGrid() = default;

	void step();

	GridData getGridData() {
		GridData data;
		data.gridCount = GRID_COUNT;
		memcpy(data.cells, grid, sizeof(CellType) * GRID_COUNT * GRID_COUNT);

		return data;
	}

  private:
	CellType grid[GRID_COUNT][GRID_COUNT];
};
