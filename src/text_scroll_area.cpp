#include "include/text_scroll_area.h"

#include <QPainter>

constexpr size_t BLOCK_SIZE = 65536;

text_scroll_area::text_scroll_area()
{
	add_block();
}

text_scroll_area::text_scroll_area(QWidget *parent) :
	QFrame(parent)
{
	add_block();
}

void text_scroll_area::add_line(const char *str, size_t size)
{
	char *line_memory = get_memory_for_line(size);
	memcpy(line_memory, str, size);
	lines.push_back(std::make_pair(line_memory, size));

	num_lines++;

	/* repaint */
	update();
}

void text_scroll_area::paintEvent(QPaintEvent *event)
{
	QFrame::paintEvent(event);
	QPainter painter(this);
	QFontMetrics font_metrics = painter.fontMetrics();
	int line_spacing = font_metrics.lineSpacing();
	int start_x = geometry().x();
	int start_y = geometry().y() + font_metrics.ascent();

	for (int i = 0; i < num_lines; i++) {
		QPointF point(start_x, start_y + line_spacing * i);
		QString text = QString::fromUtf8(lines[i].first, lines[i].second);
		painter.drawText(point, text);
	}
}

void text_scroll_area::add_block()
{
	blocks.push_back(std::make_unique<char[]>(BLOCK_SIZE));
}

char *text_scroll_area::get_memory_for_line(size_t size)
{
	if (BLOCK_SIZE - block_usage < size) {
		add_block();
		block_usage = 0;
		curr_block++;
	}

	char *line_memory = &blocks[curr_block][block_usage];
	block_usage += size;
	return line_memory;
}
