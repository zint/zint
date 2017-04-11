/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef BARCODERENDER_H
#define BARCODERENDER_H
#include <QColor>
#include <QPainter>

#include "zint.h"

namespace Zint
{

class QZint
{
private:

public:
	 enum BorderType{NO_BORDER=0, BIND=2, BOX=4};
	 enum AspectRatioMode{IgnoreAspectRatio=0, KeepAspectRatio=1, CenterBarCode=2};

public:
	QZint();
	~QZint();

	int  symbol();
	void setSymbol(int symbol);

	QString text();
	void setText(const QString & text);

	QString primaryMessage();
	void setPrimaryMessage(const QString & primaryMessage);

	void setHeight(int height);
	int height();

	void setWidth(int width);
	int width();
	
	void setOption3(int option);

	QColor fgColor();
	void setFgColor(const QColor & fgColor);

	QColor bgColor();
	void setBgColor(const QColor & bgColor);

	BorderType borderType();
	void setBorderType(BorderType border);

	int borderWidth();
	void setBorderWidth(int boderWidth);

	int pdf417CodeWords();
	void setPdf417CodeWords(int pdf417CodeWords);

	int securityLevel();
	void setSecurityLevel(int securityLevel);

	float scale();
	void setScale(float scale);
        
        void setDotSize(float dot_size);

	int mode();
	void setMode(int securityLevel);

	void setInputMode(int input_mode);

	void setWhitespace(int whitespace);

	QString error_message();

	void render(QPainter & painter, const QRectF & paintRect, AspectRatioMode mode=IgnoreAspectRatio);

	const QString & lastError();
	bool hasErrors();

	bool save_to_file(QString filename);
	
	void setHideText(bool hide);
        
        void setTargetSize(int width, int height);

private:
	void encode();
	int module_set(int y_coord, int x_coord);

private:
	int m_symbol;
	QString m_text;
	QString m_primaryMessage;
	int m_height;
	BorderType m_border;
	int m_borderWidth;
	int m_width;
	int m_securityLevel;
	int m_pdf417CodeWords;
	int m_input_mode;
	QColor m_fgColor;
	QColor m_bgColor;
	QString m_lastError;
	int m_error;
	int m_whitespace;
	zint_symbol * m_zintSymbol;
	float m_scale;
	int m_option_3;
	bool m_hidetext;
        float m_dot_size;
        int target_size_horiz;
        int target_size_vert;
};
}
#endif
