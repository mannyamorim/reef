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

/* text_scroll_area.h */
#ifndef TEXT_SCROLL_AREA_H
#define TEXT_SCROLL_AREA_H

#include <QFrame>

#include <memory>
#include <vector>

class text_scroll_area : public QFrame
{
	Q_OBJECT

public:
	text_scroll_area();
	text_scroll_area(QWidget *parent);

	void add_line(const QChar *str, size_t size);

protected:
	void paintEvent(QPaintEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	std::vector<std::unique_ptr<QChar[]>> blocks;
	std::vector<std::pair<QChar *, size_t>> lines;

	int block_usage = 0;	/* number of bytes of the current block that has been allocated */
	int curr_block = 0;	/* number of the current block */

	int num_lines = 0;	/* number of lines in total in the scroll window */
	int current_line = 0;	/* the number of the line at the top of the window */
	int selected_line = 0;	/* the number of the selected/highlighted line */

	int horiz_scroll = 0;	/* the amount of horizontal scroll that has been applied */

	void add_block();
	QChar *get_memory_for_line(size_t size);
	bool adjust_current_line(int delta);
};

#endif /* TEXT_SCROLL_AREA_H */
