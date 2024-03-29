#include "grid.hpp"

#include "util/log.hpp"

CellGrid::CellGrid() {
	clear();
}

void CellGrid::step(double dt) {
	m_accumulatedTime += dt;
	if (m_accumulatedTime > m_updateTime) {
		m_accumulatedTime -= m_updateTime;
		LOG_INFO("grid tick");
	}
}

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
