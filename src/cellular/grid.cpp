#include "grid.hpp"

#include "util/log.hpp"
#include <cstdint>

CellGrid::CellGrid() {
	clear();
}

void CellGrid::step(double dt) {
	m_accumulatedTime += dt;
	if (m_accumulatedTime > m_updateTime) {
		// reset counter
		m_accumulatedTime -= m_updateTime;

		// clear list of updated cells
		for (int row = 0; row < GRID_COUNT; row++) {
			for (int col = 0; col < GRID_COUNT; col++) {
				m_updated[col][row] = false;
			}
		}

		// update cells
		for (int row = GRID_COUNT - 1; row >= 0; row--) {
			for (int col = 0; col < GRID_COUNT; col++) {
				// check if this cell has already been updated
				if (m_updated[col][row]) {
					continue;
				}

				CellType currentType = m_grid[col][row];
				std::pair<uint32_t, uint32_t> target = {row, col}; // default to staying still

				switch (currentType) {
				case CELL_TYPE_EMPTY:
				case CELL_TYPE_SOLID:
					break;
				case CELL_TYPE_FLUID_LEFT:
					// check if can fall
					if (row < GRID_COUNT - 1 && m_grid[col][row + 1] == CellType::CELL_TYPE_EMPTY) {
						target = {row + 1, col};
						break;
					}

					// check if can move fall
					if (row < GRID_COUNT - 1 && col > 0 &&
					    m_grid[col - 1][row + 1] == CellType::CELL_TYPE_EMPTY &&
					    m_grid[col - 1][row] == CellType::CELL_TYPE_EMPTY) {
						target = {row + 1, col - 1};
						break;
					}

					// check if can move
					if (col > 0 && m_grid[col - 1][row] == CellType::CELL_TYPE_EMPTY) {
						target = {row, col - 1};
						break;
					}

					// check if can transform
					if (col < GRID_COUNT - 1 && m_grid[col + 1][row] == CellType::CELL_TYPE_EMPTY) {
						currentType = CellType::CELL_TYPE_FLUID_RIGHT;
					}
					break;
				case CELL_TYPE_FLUID_RIGHT:
					// check if can fall
					if (row < GRID_COUNT - 1 && m_grid[col][row + 1] == CellType::CELL_TYPE_EMPTY) {
						target = {row + 1, col};
						break;
					}

					// check if can move fall
					if (row < GRID_COUNT - 1 && col < GRID_COUNT - 1 &&
					    m_grid[col + 1][row + 1] == CellType::CELL_TYPE_EMPTY &&
					    m_grid[col + 1][row] == CELL_TYPE_EMPTY) {
						target = {row + 1, col + 1};
						break;
					}

					// check if can move
					if (col < GRID_COUNT - 1 && m_grid[col + 1][row] == CellType::CELL_TYPE_EMPTY) {
						target = {row, col + 1};
						break;
					}

					// check if can transform
					if (col > 0 && m_grid[col - 1][row] == CellType::CELL_TYPE_EMPTY) {
						currentType = CellType::CELL_TYPE_FLUID_LEFT;
					}
					break;
				case CELL_TYPE_SAND:
					// check if can fall
					if (row < GRID_COUNT - 1 &&
					    ((m_grid[col][row + 1] &
					      (CellType::CELL_TYPE_EMPTY | CellType::CELL_TYPE_FLUID_LEFT |
					       CellType::CELL_TYPE_FLUID_RIGHT)) != 0)) {
						target = {row + 1, col};
						break;
					}

					// check if can move fall
					if (row < GRID_COUNT - 1 && col > 0 &&
					    ((m_grid[col - 1][row + 1] &
					      (CellType::CELL_TYPE_EMPTY | CellType::CELL_TYPE_FLUID_LEFT |
					       CellType::CELL_TYPE_FLUID_RIGHT)) != 0)) {
						target = {row + 1, col - 1};
					}

					if (row < GRID_COUNT - 1 && col < GRID_COUNT - 1 &&
					    ((m_grid[col + 1][row + 1] &
					      (CellType::CELL_TYPE_EMPTY | CellType::CELL_TYPE_FLUID_LEFT |
					       CellType::CELL_TYPE_FLUID_RIGHT)) != 0)) {
						target = {row + 1, col + 1};
					}

					break;
				case CELL_TYPE_FUNGI:
					// check if can fall
					if (row < GRID_COUNT - 1 &&
					    ((m_grid[col][row + 1] &
					      (CellType::CELL_TYPE_EMPTY | CellType::CELL_TYPE_FLUID_LEFT |
					       CellType::CELL_TYPE_FLUID_RIGHT)) != 0)) {
						target = {row + 1, col};
						break;
					}
					break;
				}

				m_grid[col][row] = CellType::CELL_TYPE_EMPTY;
				m_grid[target.second][target.first] = currentType;
				m_updated[target.second][target.first] = true;
			}
		}
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
