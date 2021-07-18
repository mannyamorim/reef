/*
 * Reef - TUI Client for Git
 * Copyright (C) 2020-2021 Emmanuel Mathi-Amorim
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* block_allocator.h */
#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#include <cstdint>
#include <memory>
#include <vector>

class block_allocator
{
public:
	template<typename T>
	T *allocate(size_t num_elements)
	{
		size_t size = num_elements * sizeof(T);

		if (curr_block == -1 || BLOCK_SIZE - block_usage < size) {
			add_block();
			block_usage = 0;
			curr_block++;
		}

		uint8_t *ptr = &blocks[curr_block][block_usage];
		block_usage += size;
		return reinterpret_cast<T *>(ptr);
	}

	void clear()
	{
		blocks.clear();
		block_usage = 0;
		curr_block = -1;
	}

private:
	std::vector<std::unique_ptr<uint8_t[]>> blocks;

	constexpr static size_t BLOCK_SIZE = 65536;

	int block_usage = 0;	/* number of bytes of the current block that has been allocated */
	int curr_block = -1;	/* number of the current block */

	void add_block()
	{
		blocks.push_back(std::make_unique<uint8_t[]>(BLOCK_SIZE));
	}
};

#endif /* BLOCK_ALLOCATOR_H */
