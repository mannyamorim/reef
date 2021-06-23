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

/* version.h */
#ifndef VERSION_H
#define VERSION_H

#define REEF_VER_MAJOR 0
#define REEF_VER_MINOR 1
#define REEF_VER_PATCH 0

#define _STRINGIZE(x) #x
#define STRINGIZE(x) _STRINGIZE(x)

#define REEF_VER_DOT STRINGIZE(REEF_VER_MAJOR) "." \
                     STRINGIZE(REEF_VER_MINOR) "." \
                     STRINGIZE(REEF_VER_PATCH)

#endif /* VERSION_H */
