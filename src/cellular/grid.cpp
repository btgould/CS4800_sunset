#include "grid.hpp"

CellGrid::CellGrid() {
	clear();
}

void CellGrid::step() {}

void CellGrid::write(CellType type, uint32_t row, uint32_t col) {
	m_grid[col][row] = type;
}

void CellGrid::clear() {
	for (uint32_t i = 0; i < GRID_COUNT; i++) {
		for (uint32_t j = 0; j < GRID_COUNT; j++) {
			m_grid[i][j] = CellType::CELL_TYPE_EMPTY;
		}
	}
}
