/* dottedEdge.C
 * 
 * Copyright (C) 2009 Marcel Schumann
 * 
 * This file is part of QuEasy -- A Toolbox for Automated QSAR Model
 * Construction and Validation.
 * QuEasy is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * QuEasy is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <BALL/APPLICATIONS/QuEasyViz/dottedEdge.h>
#include <BALL/APPLICATIONS/QuEasyViz/dataItem.h>

using namespace BALL::VIEW;

DottedEdge::DottedEdge(DataItem *source_node, DataItem *dest_node)
	: Edge(source_node,dest_node,1)
 {
 }

DottedEdge::~DottedEdge()
 {
 }

//  void DottedEdge::adjust()
//  {
// 	QPointF start = mapFromItem(source_, source_position_);
// 	QPointF stop = mapFromItem(dest_, dest_position_);	
// 	QLineF line;
// 
// 	line = QLineF(mapFromItem(source_, source_->width() +1., source_->height() +1.), mapFromItem(dest_, 0, 0));
// 
// 	//prepareGeometryChange();
// 	source_point_ = line.p1();
//  	dest_point_ = line.p2();
// 	return;
// 
//  }

 void DottedEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	adjust();
	QColor color = Qt::darkGray;
	painter->setPen(QPen(color, 1, Qt::DotLine));
	QLineF line(source_point_.x(), source_point_.y(),dest_point_.x(), dest_point_.y());
	painter->drawLine(line);
}

QRectF DottedEdge::boundingRect() const
 {
     if (!source_ || !dest_)
         return QRectF();

     qreal penWidth = 1;
     qreal extra = penWidth / 2.0;

     return QRectF(source_point_, QSizeF(dest_point_.x() - source_point_.x(),dest_point_.y() -source_point_.y())).normalized().adjusted(-extra, -extra, extra, extra);
 }

void DottedEdge::setSourcePosition(QPointF newSourcePosition)
{
	source_position_ = newSourcePosition;
}

void DottedEdge::setDestPosition(QPointF newDestPosition)
{
	dest_position_ = newDestPosition;
}

QPointF DottedEdge::getSourcePosition() const
{
	return source_position_;
}

QPointF DottedEdge::getDestPosition() const
{
	return dest_position_;
}