#pragma once

#ifndef GRID_COUNT
	#define GRID_COUNT 100
#endif // !GRID_COUNT

enum CellType { CELL_TYPE_EMPTY, CELL_TYPE_SOLID, CELL_TYPE_FLUID };

class CellGrid {
  public:
	CellGrid();
	~CellGrid() = default;

	void step();

  private:
	CellType grid[GRID_COUNT][GRID_COUNT];
};
