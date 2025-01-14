/*
Copyright (C) 2014 - 2023 Evan Teran
						  evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FIXED_FONT_SELECTOR_H_20191119_
#define FIXED_FONT_SELECTOR_H_20191119_

#include <QWidget>

#include "ui_FixedFontSelector.h"

class FixedFontSelector : public QWidget {
	Q_OBJECT

public:
	explicit FixedFontSelector(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	~FixedFontSelector() override = default;

public:
	QFont currentFont();

public Q_SLOTS:
	void setCurrentFont(const QFont &font);
	void setCurrentFont(const QString &font);

private Q_SLOTS:
	void on_fontCombo_currentFontChanged(const QFont &font);
	void on_fontSize_currentIndexChanged(int index);

Q_SIGNALS:
	void currentFontChanged(const QFont &font);

private:
	Ui::FixedFontSelector ui;
};

#endif
