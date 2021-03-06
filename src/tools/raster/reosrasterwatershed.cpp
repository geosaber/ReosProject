/***************************************************************************
                      hdrasterwatershed.cpp
                     --------------------------------------
Date                 : 18-11-2018
Copyright            : (C) 2018 by Vincent Cloarec
email                : vcloarec@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "reosrasterwatershed.h"


ReosRasterWatershedMarkerFromDirection::ReosRasterWatershedMarkerFromDirection( ReosRasterWatershedFromDirectionAndDownStreamLine *parent,
    const ReosRasterWatershed::Climber &initialClimb,
    ReosRasterWatershed::Directions directionRaster,
    ReosRasterWatershed::Watershed resultRaster,
    const ReosRasterLine &excludedPixel ):
  mParent( parent ),
  mDirections( directionRaster ),
  mWatershed( resultRaster ),
  mExcludedPixel( excludedPixel )
{
  mClimberToTreat.push( initialClimb );
}

void ReosRasterWatershedMarkerFromDirection::start()
{
  while ( ( !mClimberToTreat.empty() ) && ( !isStopped() ) )
  {
    ReosRasterWatershed::Climber currentClimb = mClimberToTreat.front();
    mClimberToTreat.pop();

    bool endOfPath = true;

    for ( int i = 0; i < 3; ++i )
      for ( int j = 0; j < 3; ++j )
      {
        if ( ( i != 1 ) || ( j != 1 ) )
        {
          ReosRasterCellPos pixelToTest( currentClimb.pos.row() + i - 1, currentClimb.pos.column() + j - 1 );

          unsigned char direction = mDirections->value( pixelToTest.row(), pixelToTest.column() );

          if ( direction == ( 8 - ( i + j * 3 ) ) )
          {
            if ( !mExcludedPixel.contains( pixelToTest ) && mParent->testCell( pixelToTest ) )
            {
              mWatershed->setValue( pixelToTest.row(), pixelToTest.column(), 1 );
              double dl = 0;
              if ( direction % 2 == 0 )
              {
                dl = sqrt( 2 );
              }
              else
                dl = 1;

              if ( mClimberToTreat.size() < mMaxClimberStored )
                mClimberToTreat.push( ReosRasterWatershed::Climber( pixelToTest, currentClimb.lengthPath + dl ) );
              else


                endOfPath &= false;
            }
          }
        }
      }

    if ( endOfPath )
    {
      mParent->proposeEndOfPath( currentClimb );
    }

    if ( mClimberToTreat.empty() )
    {
      bool pixelAvailable;
      ReosRasterWatershed::Climber climb = mParent->getClimberFromPool( pixelAvailable );
      if ( pixelAvailable )
        mClimberToTreat.push( climb );
    };

    if ( isStopAsked() )
      stop( true );

  }

}

ReosRasterWatershedFromDirectionAndDownStreamLine::ReosRasterWatershedFromDirectionAndDownStreamLine( ReosRasterWatershed::Directions rasterDirection,
    const ReosRasterLine &line ):
  mDirections( rasterDirection ), mDownstreamLine( line )
{
  mWatershed = std::make_shared<ReosRasterMemory<unsigned char> >( mDirections->rowCount(), mDirections->columnCount() ) ;
  mWatershed->reserveMemory();
  mWatershed->fill( 0 );

  for ( unsigned i = 0; i < mDownstreamLine.cellCount(); ++i )
  {
    ReosRasterCellPos pix = mDownstreamLine.cellPosition( i );
    mPoolCellsToTreat.push_back( ReosRasterWatershed::Climber( pix ) );
    mWatershed->setValue( pix.row(), pix.column(), 1 );
  }

  setMaxProgession( int( mPoolCellsToTreat.size() ) );
  unsigned halfPos = mDownstreamLine.cellCount() / 2;
  mFirstCell = mDownstreamLine.cellPosition( halfPos );

}

ReosRasterWatershedFromDirectionAndDownStreamLine::ReosRasterWatershedFromDirectionAndDownStreamLine(
  ReosRasterWatershed::Directions rasterDirection,
  const ReosRasterLine &line,
  ReosRasterTestingCell *testingCell ):
  ReosRasterWatershedFromDirectionAndDownStreamLine( rasterDirection, line )
{
  mTestingCell.reset( testingCell );
}

ReosRasterWatershed::Climber ReosRasterWatershedFromDirectionAndDownStreamLine::getClimberFromPool( bool &available )
{
  ReosRasterWatershed::Climber climber;
  mMutexClimber.lock();
  if ( mPoolCellsToTreat.empty() )
  {
    available = false;
  }
  else
  {
    available = true;
    climber = mPoolCellsToTreat.front();
    mPoolCellsToTreat.pop_front();
    mCounter++;
    setCurrentProgression( mCounter );
  }
  mMutexClimber.unlock();
  return climber;
}

void ReosRasterWatershedFromDirectionAndDownStreamLine::addClimberInPool( const ReosRasterWatershed::Climber &climb )
{
  mMutexClimber.lock();
  mPoolCellsToTreat.push_front( climb );
  mMutexClimber.unlock();
}

void ReosRasterWatershedFromDirectionAndDownStreamLine::proposeEndOfPath( ReosRasterWatershed::Climber climber )
{
  mMutexEndOfPath.lock();
  if ( climber.lengthPath > mEndOfLongerPath.lengthPath )
  {
    mEndOfLongerPath = climber;
  }
  mMutexEndOfPath.unlock();
}

ReosRasterWatershed::Watershed ReosRasterWatershedFromDirectionAndDownStreamLine::watershed() const {return mWatershed;}

ReosRasterCellPos ReosRasterWatershedFromDirectionAndDownStreamLine::fisrtCell() const {return mFirstCell;}

ReosRasterCellPos ReosRasterWatershedFromDirectionAndDownStreamLine::endOfLongerPath() const {return mEndOfLongerPath.pos;}

bool ReosRasterWatershedFromDirectionAndDownStreamLine::testCell( const ReosRasterCellPos &cell ) const
{
  if ( mTestingCell )
    return mTestingCell->testCell( cell );
  else
    return true;
}

void ReosRasterWatershedFromDirectionAndDownStreamLine::start()
{

  unsigned nbThread = std::thread::hardware_concurrency() - 1;


  mThreads.clear();
  mJobs.clear();

  for ( unsigned i = 0; i < nbThread; ++i )
  {
    bool pixelAvailable;
    ReosRasterWatershed::Climber pix = getClimberFromPool( pixelAvailable );
    if ( pixelAvailable )
    {
      ReosRasterWatershedMarkerFromDirection *cal = new ReosRasterWatershedMarkerFromDirection( this, pix, mDirections, mWatershed, mDownstreamLine );
      mJobs.emplace_back( cal );
      mThreads.emplace_back( ReosProcess::processStart, cal );
    }
  }

  for ( auto &&t : mThreads )
  {
    t.join();
  }

  mJobs.clear();
  mThreads.clear();
}

void ReosRasterWatershedFromDirectionAndDownStreamLine::stopAsSoonAsPossible( bool b )
{
  for ( auto &calc : mJobs )
    calc->stopAsSoonAsPossible( b );
}

ReosRasterWatershedToVector::ReosRasterWatershedToVector( ReosRasterWatershed::Watershed rasterWatershed,
    const ReosRasterExtent &extent,
    const ReosRasterCellPos &cellInWatershed ):
  mRasterWatershed( rasterWatershed ), mExtent( extent )
{
  bool findLimit = false;
  int Columnlimite = cellInWatershed.column();

  while ( ( !findLimit ) && ( Columnlimite > -1 ) )
  {
    Columnlimite--;
    if ( rasterWatershed->value( cellInWatershed.row(), Columnlimite ) != 1 )
    {
      findLimit = true;
    }
  }

  Columnlimite++;

  QPoint startingPoint = QPoint( cellInWatershed.row(), Columnlimite );
  QPoint origin( 0, -1 );

  QVector<QPoint> endLine;
  endLine.append( startingPoint );

  mWatershedTrace = std::unique_ptr<ReosRasterTraceBetweenCellsUniqueValue<unsigned char>>(
                      new ReosRasterTraceBetweenCellsUniqueValue<unsigned char>( rasterWatershed.get(), 1, startingPoint, origin, endLine, mEliminationPoint ) );

  setMaxProgession( 0 );
}

const QPolygonF ReosRasterWatershedToVector::watershed() const
{
  QPolygonF vectorWatershed;
  const QPolygon &rasterWatershed = mWatershedTrace->trace();
  vectorWatershed.resize( rasterWatershed.count() );

  for ( int i = 0; i < rasterWatershed.count(); ++i )
    vectorWatershed[i] = mExtent.interCellToMap( rasterWatershed.at( i ) );

  return vectorWatershed;
}

void ReosRasterWatershedToVector::start()
{
  mWatershedTrace->startTracing();
}

ReosRasterWatershedTraceDownstream::ReosRasterWatershedTraceDownstream( ReosRasterWatershed::Directions directionRaster, const ReosRasterLine stopLine, const ReosRasterExtent &extent, const ReosRasterCellPos &startPos ):
  mDirectionRaster( directionRaster ),
  mStopLine( stopLine ),
  mEmpriseRaster( extent ),
  mPos( startPos )
{}

ReosRasterWatershedTraceDownstream::ReosRasterWatershedTraceDownstream( ReosRasterWatershed::Directions directionRaster, const QPolygonF &polyLimit, const ReosRasterExtent &extent, const ReosRasterCellPos &startPos ):
  mDirectionRaster( directionRaster ),
  mEmpriseRaster( extent ),
  mPos( startPos ),
  mPolyLimit( polyLimit )
{}

void ReosRasterWatershedTraceDownstream::start()
{
  unsigned char lastDir = 4;
  unsigned char dir = mDirectionRaster->value( mPos.row(), mPos.column() );
  QPointF posMap = mEmpriseRaster.cellCenterToMap( mPos );
  bool pointIsInPolyLimit = true;
  bool isStopLine = false;
  bool testIsInPolygon = !mPolyLimit .isEmpty();

  while ( ( !isStopLine ) && ( dir != 4 ) && ( dir != 9 ) && ( !isStopAsked() ) && pointIsInPolyLimit )
  {
    if ( dir != lastDir )
      mResultPolyline.append( posMap );
    lastDir = dir;
    mPos = mPos.neighbourWithDirection( dir );
    posMap = mEmpriseRaster.cellCenterToMap( mPos );

    if ( testIsInPolygon )
      pointIsInPolyLimit = mPolyLimit.containsPoint( posMap, Qt::OddEvenFill );
    if ( mStopLine.cellCount() != 0 )
      isStopLine = mStopLine.contains( mPos );
    dir = mDirectionRaster->value( mPos.row(), mPos.column() );
  }
  mResultPolyline.append( mEmpriseRaster.cellCenterToMap( mPos ) );
}

QPolygonF ReosRasterWatershedTraceDownstream::resultPolyline() const
{
  return mResultPolyline;
}
