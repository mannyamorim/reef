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

/* cpp_curses.h */
#ifndef CPP_CURSES_H
#define CPP_CURSES_H

#include <cstdarg>
#include <exception>

#include <curses.h>

#if NCURSES_MOUSE_VERSION || PDCURSES
#define REEF_MOUSE_SUPPORTED
#endif /* NCURSES_MOUSE_VERSION || PDCURSES */

namespace cpp_curses
{
	class curses_error : public std::exception
	{
	public:
		curses_error(const char *error) : error(error) {}

		virtual const char *what() const noexcept override
		{
			return error;
		}

	private:
		const char *error;
	};

	class curses_mode
	{
	public:
		curses_mode()
		{
			initscr();	/* start curses mode */
			raw();		/* line buffering disabled */
			noecho();	/* don't echo() while we do getch */
			refresh();	/* refresh screen before creating the windows */

#ifdef NCURSES_MOUSE_VERSION
			mousemask(ALL_MOUSE_EVENTS, NULL);
#endif /* NCURSES_MOUSE_VERSION */
#ifdef PDCURSES
			mouse_on(ALL_MOUSE_EVENTS);
#endif /* PDCURSES */
		}

		~curses_mode()
		{
			endwin();	/* end curses mode */
		}

		static void _doupdate()
		{
			if (doupdate() == ERR)
				throw curses_error("doupdate failed");
		}
	};

#ifdef REEF_MOUSE_SUPPORTED
	class mouse_event
	{
	public:
		int x, y;
		bool button_1_clicked : 1;
		bool button_2_clicked : 1;
		bool button_1_double_clicked : 1;
		bool button_2_double_clicked : 1;
		bool scroll_up : 1;
		bool scroll_down : 1;

		mouse_event()
		{
#ifdef NCURSES_MOUSE_VERSION
			MEVENT event;
			if (getmouse(&event) == ERR)
				throw curses_error("getmouse failed");

			x = event.x;
			y = event.y;
			button_1_clicked = event.bstate & BUTTON1_CLICKED;
			button_2_clicked = event.bstate & BUTTON2_CLICKED;
			button_1_double_clicked = event.bstate & BUTTON1_DOUBLE_CLICKED;
			button_2_double_clicked = event.bstate & BUTTON2_DOUBLE_CLICKED;
#if NCURSES_MOUSE_VERSION > 1
			scroll_up = event.bstate & BUTTON4_PRESSED;
			scroll_down = event.bstate & BUTTON5_PRESSED;
#endif /* NCURSES_MOUSE_VERSION > 1 */
#endif /* NCURSES_MOUSE_VERSION */
#ifdef PDCURSES
			request_mouse_pos();
			MOUSE_STATUS status = Mouse_status;
			x = MOUSE_X_POS;
			y = MOUSE_Y_POS;
			button_1_clicked = BUTTON_CHANGED(1) && (BUTTON_STATUS(1) == BUTTON_CLICKED);
			button_2_clicked = BUTTON_CHANGED(2) && (BUTTON_STATUS(2) == BUTTON_CLICKED);
			button_1_double_clicked = BUTTON_CHANGED(1) && (BUTTON_STATUS(1) == BUTTON_DOUBLE_CLICKED);
			button_2_double_clicked = BUTTON_CHANGED(2) && (BUTTON_STATUS(2) == BUTTON_DOUBLE_CLICKED);
			scroll_up = MOUSE_WHEEL_UP;
			scroll_down = MOUSE_WHEEL_DOWN;
#endif /* PDCURSES */
		}
	};
#endif /* REEF_MOUSE_SUPPORTED */

	class window
	{
	public:
		window(int nlines, int ncols, int begin_y, int begin_x)
		{
			win = newwin(nlines, ncols, begin_y, begin_x);
			if (!win)
				throw curses_error("curses window creation failed");
		}

		~window()
		{
			delwin(win);
		}
		
		inline void print(const char *fmt, ...)
		{
			va_list args;

			va_start(args, fmt);
			const int retval = vw_printw(win, fmt, args);
			va_end(args);

			if (retval == ERR)
				throw curses_error("curses wprintw failed");
		}

		inline void set_keypad(bool value)
		{
			if (keypad(win, value) == ERR)
				throw curses_error("curses keypad function error");
		}

		inline void set_idlok(bool value)
		{
			if (idlok(win, value) == ERR)
				throw curses_error("curses window setting (idlok) failed");
		}
		
		inline void set_scrollok(bool value)
		{
			if (scrollok(win, value) == ERR)
				throw curses_error("curses window setting (scrollok) failed");
		}

		inline void mv(int y, int x)
		{
			if (mvwin(win, y, x) == ERR)
				throw curses_error("curses move window failed");
		}

		inline void resize(int nlines, int ncols)
		{
			if (wresize(win, nlines, ncols) == ERR)
				throw curses_error("curses wresize failed");
		}

		inline void refresh()
		{
			if (wrefresh(win) == ERR)
				throw curses_error("curses window refresh failed");
		}

		inline void noutrefresh()
		{
			if (wnoutrefresh(win) == ERR)
				throw curses_error("curses wnoutrefresh failed");
		}

		inline void redraw()
		{
			if (redrawwin(win) == ERR)
				throw curses_error("curses redrawwin failed");
		}

		inline void clear()
		{
			if (wclear(win) == ERR)
				throw curses_error("curses wclear failed");
		}

		inline void _mvaddchnstr(int y, int x, const chtype *ch, int n)
		{
			if (mvwaddchnstr(win, y, x, ch, n) == ERR)
				throw curses_error("curses mvwaddchnstr failed");
		}

		inline void _scrl(int n)
		{
			if (wscrl(win, n) == ERR)
				throw curses_error("curses wscrl failed");
		}

		inline int _getch()
		{
			const int res = wgetch(win);
			if (res == ERR)
				throw curses_error("curses wgetch failed");
			return res;
		}

		inline int _clrtoeol()
		{
			const int res = wclrtoeol(win);
			if (res == ERR)
				throw curses_error("curses window wclrtoeol");
			return res;
		}

		inline int _move(int y, int x)
		{
			const int res = wmove(win, y, x);
			if (res == ERR)
				throw curses_error("curses window wmove");
			return res;
		}

	private:
		WINDOW *win;
	};
}

#endif /* CPP_CURSES_H */
