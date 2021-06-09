/*
 * Reef - TUI Client for Git
 * Copyright (C) 2020 Emmanuel Mathi-Amorim
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

/* error.h */
#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <string>

class reef_error : public std::exception
{
public:
	reef_error(std::string &&err_msg) :
		err_msg(std::move(err_msg))
	{}

	virtual const char *what() const noexcept override
	{
		return err_msg.c_str();
	}

private:
	std::string err_msg;
};

#endif /* ERROR_H */
