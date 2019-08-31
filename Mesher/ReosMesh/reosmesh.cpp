/***************************************************************************
                      reosmesh.cpp
                     --------------------------------------
Date                 : 01-04-2019
Copyright            : (C) 2019 by Vincent Cloarec
email          : vcloarec at gmail dot com   /  projetreos at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "reosmesh.h"


Vertex::~Vertex() {}

void *Vertex::graphicPointer() const {
    return mGraphic;
}


ReosMesh::~ReosMesh() {}

VertexPointer ReosMesh::vertex(double x, double y) const{
    return vertex(x,y,mTolerance);
}

bool ReosMesh::isDirty() const {return mDirty;}



MeshIO::~MeshIO() {}

Segment::Segment(VertexPointer v1, VertexPointer v2):mVertex1(v1),mVertex2(v2)
{}

VertexPointer Segment::first() const {return mVertex1;}

VertexPointer Segment::second() const {return mVertex2;}



bool Face::isVertexContained(VertexPointer vertex) const
{
    bool found=false;
    int i=0;
    while( !found && i<verticesCount())
    {
        found= vertex == vertexPointer(i);
        if (!found)
            ++i;
    }

    return found;
}

std::vector<double> Face::faceCentroid()
{
    double x=0;
    double y=0;
    int n=verticesCount();
    for (int i=0;i<n;++i)
    {
        x+=vertexPointer(i)->x();
        y+=vertexPointer(i)->y();
    }

    std::vector<double> centroid{x/n,y/n};

    return centroid;
}

void Vertex::setGraphicPointer(void *pointer)
{
    mGraphic=pointer;
}

void Vertex::setZUserDefined()
{
    mZUserDefined_=true;
}

bool Vertex::isZUserDefined() const
{
    return mZUserDefined_;
}

VertexZSpecifierSimple::VertexZSpecifierSimple(const VertexPointer associatedVertex,double z):VertexZSpecifier(associatedVertex),mZValue(z)
{

}

double VertexZSpecifierSimple::getZValue() const
{
    return mZValue;
}

VertexZSpecifier::~VertexZSpecifier(){}

VertexZSpecifierOtherVertexAndSlope::VertexZSpecifierOtherVertexAndSlope(VertexPointer associatedVertex, VertexPointer otherVertex, double slope):
    VertexZSpecifier(associatedVertex),mOtherVertex(otherVertex),mSlope(slope)
{

}

double VertexZSpecifierOtherVertexAndSlope::getZValue() const
{
    if(mOtherVertex)
        return mOtherVertex->z()-mSlope*mOtherVertex->distanceFrom(*mAssociatedVertex);
    else
        return 0;

}
