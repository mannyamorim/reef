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

#include "graph_delegate.h"

#include "graph.h"

constexpr QColor graph_colors[] = {
	QColor(0, 0, 0),
	QColor(128, 0, 0),
	QColor(0, 128, 0),
	QColor(0, 0, 128),
	QColor(128, 128, 0),
	QColor(128, 0, 128),
	QColor(0, 128, 128)
};

/* width to height ratio for the characters */
constexpr qreal character_aspect_ratio = 0.6;

/* width of the pen strokes */
constexpr qreal stroke_width = 1.0;

void graph_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.data().canConvert<QByteArray>()) {
		/* get the graph string from the model */
		QByteArray arr = qvariant_cast<QByteArray>(index.data());
		graph_char *graph_str = reinterpret_cast<graph_char *>(arr.data());
		size_t graph_len = arr.length() / sizeof(graph_char);

		/* save the painter settings so we can restore later */
		painter->save();

		/* prepare the paint settings */
		QBrush brush = QBrush(Qt::SolidPattern);
		QPen pen = QPen(brush, stroke_width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
		painter->setBrush(brush);
		painter->setClipRect(option.rect);
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setPen(pen);

		/* calculate the sizes */
		const qreal height = option.rect.height();
		const qreal width = height * character_aspect_ratio;
		const qreal half_height = height / 2.0;
		const qreal half_width = width / 2.0;
		const qreal radius = (qreal)half_width / 2.0;
		painter->translate(option.rect.x(), option.rect.y());

		/* paint the graph chars */
		for (size_t i = 0; i < graph_len; i++) {
			/* set the color */
			pen.setColor(graph_colors[graph_str[i].color]);
			painter->setPen(pen);

			/* draw the character */
			switch (graph_str[i].flags) {
			case G_MARK:
				painter->drawEllipse(QPointF(half_width, half_height), radius, radius);
				break;
			case G_INITIAL:
				painter->drawEllipse(QPointF(half_width, half_height), radius, radius);
				break;
			default:
				if (graph_str[i].flags & G_LEFT)
					painter->drawLine(QPointF(0, half_height), QPointF(half_width, half_height));
				if (graph_str[i].flags & G_RIGHT)
					painter->drawLine(QPointF(half_width, half_height), QPointF(width, half_height));
				if (graph_str[i].flags & G_UPPER)
					painter->drawLine(QPointF(half_width, 0), QPointF(half_width, half_height));
				if (graph_str[i].flags & G_LOWER)
					painter->drawLine(QPointF(half_width, half_height), QPointF(half_width, height));
				break;
			}

			/* move to the next space */
			painter->translate(width, 0.0);
		}

		/* restore the painter settings */
		painter->restore();
	}
}
