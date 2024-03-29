#include "grid.hpp"

#include "util/log.hpp"
#include <cstdint>

CellGrid::CellGrid() {
	clear();
}

void CellGrid::step(double dt) {
	m_accumulatedTime += dt;
	if (m_accumulatedTime > m_updateTime) {
		m_accumulatedTime -= m_updateTime;

		for (int row = GRID_COUNT - 1; row >= 0; row--) {
			for (int col = 0; col < GRID_COUNT; col++) {
				CellType currentType = m_grid[col][row];
				CellType* target = &m_grid[col][row]; // default to staying still

				switch (m_grid[col][row]) {
				case CELL_TYPE_EMPTY:
				case CELL_TYPE_SOLID:
					break;
				case CELL_TYPE_FLUID_LEFT:
					// check if can fall
					if (row < GRID_COUNT - 1 && m_grid[col][row + 1] == CellType::CELL_TYPE_EMPTY) {
						target = &m_grid[col][row + 1];
						break;
					}

					// check if can move fall
					if (row < GRID_COUNT - 1 && col > 0 &&
					    m_grid[col - 1][row + 1] == CellType::CELL_TYPE_EMPTY) {
						target = &m_grid[col - 1][row + 1];
						break;
					}

					// check if can move
					if (col > 0 && m_grid[col - 1][row] == CellType::CELL_TYPE_EMPTY) {
						target = &m_grid[col - 1][row];
						break;
					}

					// check if can transform
					if (col < GRID_COUNT - 1 && m_grid[col + 1][row] == CellType::CELL_TYPE_EMPTY) {
						currentType = CellType::CELL_TYPE_FLUID_RIGHT;
					}
					break;

				case CELL_TYPE_FLUID_RIGHT:
					break;
				}

				m_grid[col][row] = CellType::CELL_TYPE_EMPTY;
				*target = currentType;
			}

			for (int col = GRID_COUNT - 1; col >= 0; col--) {
				CellType currentType = m_grid[col][row];
				CellType* target = &m_grid[col][row]; // default to staying still

				switch (currentType) {

				case CELL_TYPE_EMPTY:
				case CELL_TYPE_SOLID:
				case CELL_TYPE_FLUID_LEFT:
					break;
				case CELL_TYPE_FLUID_RIGHT:
					// check if can fall
					if (row < GRID_COUNT - 1 && m_grid[col][row + 1] == CellType::CELL_TYPE_EMPTY) {
						target = &m_grid[col][row + 1];
						break;
					}

					// check if can move fall
					if (row < GRID_COUNT - 1 && col < GRID_COUNT - 1 &&
					    m_grid[col + 1][row + 1] == CellType::CELL_TYPE_EMPTY) {
						target = &m_grid[col + 1][row + 1];
						break;
					}

					// check if can move
					if (col < GRID_COUNT - 1 && m_grid[col + 1][row] == CellType::CELL_TYPE_EMPTY) {
						target = &m_grid[col + 1][row];
						break;
					}

					// check if can transform
					if (col > 0 && m_grid[col - 1][row] == CellType::CELL_TYPE_EMPTY) {
						currentType = CellType::CELL_TYPE_FLUID_LEFT;
					}
					break;
				}

				m_grid[col][row] = CellType::CELL_TYPE_EMPTY;
				*target = currentType;
			}
		}
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
