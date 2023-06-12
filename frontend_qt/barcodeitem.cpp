/***************************************************************************
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bogdan@licentia.eu                                                    *
 *   Copyright (C) 2009-2023 by Robin Stuart <rstuart114@gmail.com>        *
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
/* vim: set ts=4 sw=4 et : */

//#include <QDebug>
#include "barcodeitem.h"

BarcodeItem::BarcodeItem()
        : QGraphicsItem()
{
    w = 693;
    h = 378; // Default widget size when created
}

BarcodeItem::~BarcodeItem()
{
}

void BarcodeItem::setSize(int width, int height) {
    w = width;
    h = height;
}

void BarcodeItem::setColor(const QColor& color) {
    m_color = color;
}

QRectF BarcodeItem::boundingRect() const
{
    return QRectF(0, 0, w, h);
}

void BarcodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QRectF painterRect = boundingRect();
    if (m_color.isValid()) {
        painter->fillRect(painterRect, m_color);
    }
    bc.render(*painter, painterRect);
}
