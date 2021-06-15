#include "include/text_scroll_area.h"

#include <QKeyEvent>
#include <QPainter>

#include "preferences.h"

constexpr size_t BLOCK_SIZE = 65536;
constexpr int WHEEL_DEGREES_TO_ANGLE_DELTA = 8;
constexpr int WHEEL_STEPS_TO_DEGREES = 15;

text_scroll_area::text_scroll_area()
{
	add_block();
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

text_scroll_area::text_scroll_area(QWidget *parent) :
	QFrame(parent)
{
	add_block();
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
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
	int num_lines_in_screen = geometry().height() / line_spacing;
	int num_lines_to_draw = std::min(num_lines - current_line, num_lines_in_screen);

	for (int i = 0; i < num_lines_to_draw; i++) {
		QPointF point(start_x, start_y + line_spacing * i);
		QString text = QString::fromUtf8(lines[i + current_line].first, lines[i + current_line].second);
		painter.drawText(point, text);
	}
}

void text_scroll_area::keyPressEvent(QKeyEvent *event)
{
	switch(event->key()) {
	case Qt::Key_Up:
		adjust_current_line(-1);
		break;
	case Qt::Key_Down:
		adjust_current_line(+1);
		break;
	case Qt::Key_PageUp:
		adjust_current_line(-preferences::lines_page_up_down);
		break;
	case Qt::Key_PageDown:
		adjust_current_line(+preferences::lines_page_up_down);
		break;
	}

	update();
}

void text_scroll_area::wheelEvent(QWheelEvent *event)
{
	QPoint num_pixels = event->pixelDelta();
	QPoint num_degrees = event->angleDelta() / WHEEL_DEGREES_TO_ANGLE_DELTA;

	int scroll_delta;

	if (!num_pixels.isNull()) {
		scroll_delta = num_pixels.y();
	} else if (!num_degrees.isNull()) {
		QPoint num_steps = num_degrees / -WHEEL_STEPS_TO_DEGREES;
		scroll_delta = num_steps.y() * preferences::lines_mouse_scroll;
	} else {
		return;
	}

	bool changed = adjust_current_line(scroll_delta);
	if (changed) {
		event->accept();
		update();
	} else {
		event->ignore();
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

bool text_scroll_area::adjust_current_line(int delta)
{
	int new_line = current_line + delta;

	if (new_line >= num_lines)
		new_line = num_lines - 1;

	if (new_line < 0)
		new_line = 0;

	bool changed = (new_line != current_line);
	current_line = new_line;
	return changed;
}
