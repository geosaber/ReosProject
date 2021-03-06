#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


#include "../../Mesher/ReosMesh/reosvertexzspecifier.h"
#include "../../Mesher/ReosMesh/reosmeshgenerator.h"



using namespace testing;

class VertexZSpecifierTesting: public Test
{
  public:


    VertexBasic vert1 = VertexBasic( 0, 0 );
    VertexBasic vert2 = VertexBasic( 5, 0 );
    VertexBasic vert3 = VertexBasic( 0, 5 );
    VertexBasic vert4 = VertexBasic( 2.5, 0 );


    ReosVertexZSpecifierSimpleFactory simpleZSpecifierFactory;
    ReosVertexZSpecifierOtherVertexAndSlopeFactory slopeZSpecifierFactory;
    ReosVertexZSpecifierOtherVertexAndGapFactory gapZSpecifierFactory;
    ReosVertexZSpecifierInterpolationFactory interpolationZSpecifierFactory;

    // Test interface
  protected:
    void SetUp() override
    {
      simpleZSpecifierFactory.setZValue( 5 );
      vert1.setZSpecifier( simpleZSpecifierFactory );
    }

    void TEST_createVertexSlopeSpecifier()
    {
      slopeZSpecifierFactory.setSlope( 0.05 );
      slopeZSpecifierFactory.setOtherVertex( &vert1 );
      vert2.setZSpecifier( slopeZSpecifierFactory );

      ASSERT_THAT( abs( vert2.z() - 5.25 ), Lt( std::numeric_limits<double>::min() ) );
    }

    void TEST_createVertexGapSpecifier()
    {
      gapZSpecifierFactory.setGap( 0.05 );
      gapZSpecifierFactory.setOtherVertex( &vert1 );
      vert2.setZSpecifier( gapZSpecifierFactory );

      ASSERT_THAT( abs( vert2.z() - 5.05 ), Lt( std::numeric_limits<double>::min() ) );
    }

    void TEST_createVertexInterpolationSpecifier()
    {
      simpleZSpecifierFactory.setZValue( 4 );
      vert3.setZSpecifier( simpleZSpecifierFactory );

      simpleZSpecifierFactory.setZValue( 18 );
      vert2.setZSpecifier( simpleZSpecifierFactory );

      interpolationZSpecifierFactory.setExtremitiesVertices( &vert3, &vert2 );
      vert1.setZSpecifier( interpolationZSpecifierFactory );


      ASSERT_THAT( abs( vert1.z() - 11 ), Lt( std::numeric_limits<double>::min() ) );

    }

};


TEST_F( VertexZSpecifierTesting, verticesDistance )
{

  auto vertA = VertexBasic( 0, 0 );
  auto vertB = VertexBasic( 3, 4 );

  ASSERT_THAT( abs( vertA.distanceFrom( vertB ) - 5 ), Lt( std::numeric_limits<double>::min() ) );
}

TEST_F( VertexZSpecifierTesting, createSimpleSpecifierDefault )
{

  vert1.setZSpecifier( simpleZSpecifierFactory );

  ASSERT_THAT( vert1.z(), Eq( 5 ) );
}

TEST_F( VertexZSpecifierTesting, createSimpleSpecifierWithValue )
{


  simpleZSpecifierFactory.setZValue( 5 );
  vert1.setZSpecifier( simpleZSpecifierFactory );

  ASSERT_THAT( vert1.z(), Eq( 5 ) );
}

TEST_F( VertexZSpecifierTesting, createVertexSlopeSpecifier )
{

  TEST_createVertexSlopeSpecifier();

}

TEST_F( VertexZSpecifierTesting, slopeSpecifier_changeOtherVertexZValue )
{

  TEST_createVertexSlopeSpecifier();

  simpleZSpecifierFactory.setZValue( 10 );
  vert1.setZSpecifier( simpleZSpecifierFactory );

  ASSERT_THAT( abs( vert2.z() - 10.25 ), Lt( std::numeric_limits<double>::min() ) );

}

TEST_F( VertexZSpecifierTesting, slopeSpecifier_changeOtherVertexPosition )
{

  TEST_createVertexSlopeSpecifier();

  vert1.setPosition( 4, 0 );

  ASSERT_THAT( abs( vert2.z() - 5.05 ), Lt( std::numeric_limits<double>::min() ) );

}

TEST_F( VertexZSpecifierTesting, createVertexGapSpecifier )
{

  TEST_createVertexGapSpecifier();
}


TEST_F( VertexZSpecifierTesting, createVertexGapSpecifier_changeOtherVertexZValue )
{

  TEST_createVertexGapSpecifier();

  simpleZSpecifierFactory.setZValue( 10 );
  vert1.setZSpecifier( simpleZSpecifierFactory );

  ASSERT_THAT( abs( vert2.z() - 10.05 ), Lt( std::numeric_limits<double>::min() ) );
}

TEST_F( VertexZSpecifierTesting, createVertexGapSpecifier_compatibility )
{

  TEST_createVertexGapSpecifier();

  gapZSpecifierFactory.setOtherVertex( &vert2 );
  EXPECT_FALSE( vert1.isSpecifierIsCompatible( gapZSpecifierFactory ) );

  gapZSpecifierFactory.setOtherVertex( &vert3 );
  EXPECT_TRUE( vert1.isSpecifierIsCompatible( gapZSpecifierFactory ) );
}

TEST_F( VertexZSpecifierTesting, createInterpolatorSpecifier )
{

  TEST_createVertexInterpolationSpecifier();
}


TEST_F( VertexZSpecifierTesting, selfInterpolateVertex )
{

  simpleZSpecifierFactory.setZValue( 18 );
  vert2.setZSpecifier( simpleZSpecifierFactory );

  simpleZSpecifierFactory.setZValue( 10 );
  vert1.setZSpecifier( simpleZSpecifierFactory );

  interpolationZSpecifierFactory.setExtremitiesVertices( &vert1, &vert2 );
  vert1.setZSpecifier( interpolationZSpecifierFactory );


  ASSERT_THAT( abs( vert1.z() - 10 ), Lt( std::numeric_limits<double>::min() ) );

}

TEST_F( VertexZSpecifierTesting, removeInterpolateVertices )
{
  double z1 = 7;
  simpleZSpecifierFactory.setZValue( z1 );
  vert1.setZSpecifier( simpleZSpecifierFactory );

  double z3 = 7;
  simpleZSpecifierFactory.setZValue( z3 );
  vert3.setZSpecifier( simpleZSpecifierFactory );

  std::vector<VertexBasic *> verticesInterpolated;
  double distance = vert1.distanceFrom( vert3 );
  size_t count = 3;
  double step = distance / ( count + 1 );

  interpolationZSpecifierFactory.setExtremitiesVertices( &vert1, &vert3 );

  for ( size_t i = 0; i < count; ++i )
  {
    verticesInterpolated.push_back( new VertexBasic( 0, ( i + 1 )*step ) );
    verticesInterpolated[i]->setZSpecifier( interpolationZSpecifierFactory );
  }

  EXPECT_TRUE( equality( verticesInterpolated[1]->z(), 7 ) );

  verticesInterpolated[0]->hasToBeRemoved();
  delete verticesInterpolated[0];

  double z = verticesInterpolated[1]->z();
  EXPECT_TRUE( equality( verticesInterpolated[1]->z(), 7 ) );
}


TEST_F( VertexZSpecifierTesting, manyInterpolateVertices )
{

  double z1 = 4;
  simpleZSpecifierFactory.setZValue( z1 );
  vert1.setZSpecifier( simpleZSpecifierFactory );

  double z3 = 18;
  simpleZSpecifierFactory.setZValue( z3 );
  vert3.setZSpecifier( simpleZSpecifierFactory );

  double distance = vert1.distanceFrom( vert3 );


  std::vector<VertexBasic *> verticesInterpolated;
  size_t count = 9;
  double step = distance / ( count + 1 );
  double gapPerStep = ( z3 - z1 ) / ( count + 1 );

  interpolationZSpecifierFactory.setExtremitiesVertices( &vert1, &vert3 );

  for ( size_t i = 0; i < count; ++i )
  {
    verticesInterpolated.push_back( new VertexBasic( 0, ( i + 1 )*step ) );
    verticesInterpolated[i]->setZSpecifier( interpolationZSpecifierFactory );
  }


  //******************************************
  /*
   *     Vert1  [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z      4     I      I      I      I      I      I      I      I      I       18
   *Dist     0.5   0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi          5.4    6.8    8.2    9.6    11     12.4   13.8   15.2   16.6
   * */

  //add a gap specifier from interpolated point
  auto vert = VertexBasic( 1, 1 );
  gapZSpecifierFactory.setGap( -1.1 );
  gapZSpecifierFactory.setOtherVertex( verticesInterpolated[5] );
  vert.setZSpecifier( gapZSpecifierFactory );
  EXPECT_TRUE( equality( vert.z(), 11.3 ) );

  for ( size_t i = 0; i < count; ++i )
  {
    EXPECT_TRUE( equality( verticesInterpolated[i]->z(), ( z1 + ( i + 1 )*gapPerStep ) ) );
  }

  //move one of the point which interpolation is base on
  vert1.setPosition( 0, -5 );

  //******************************************
  /*
   *     Vert1  [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z      4     I      I      I      I      I      I      I      I      I       18
   *Dist     5.5   0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi
   * */

  double totalGap = vert3.z() - vert1.z();
  distance = vert1.distanceFrom( vert3 );
  for ( size_t i = 0; i < count; ++i )
  {
    double di = vert1.distanceFrom( ( *verticesInterpolated[i] ) );
    double zi = di / distance * totalGap + vert1.z();
    EXPECT_TRUE( equality( verticesInterpolated[i]->z(), zi ) );
  }


  //replace the point which interpolation is base on
  vert1.setPosition( 0, 0 );
  //******************************************
  /*
   *     Vert1  [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z      4     I      I      I      I      I      I      I      I      I       18
   *Dist     0.5   0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi          5.4    6.8    8.2    9.6    11     12.4   13.8   15.2   16.6
   * */

  EXPECT_TRUE( equality( vert.z(), 11.3 ) );

  //change the specifier of one interpolated point
  simpleZSpecifierFactory.setZValue( -8 );
  verticesInterpolated[5]->setZSpecifier( simpleZSpecifierFactory );

  //******************************************
  /*
   *     Vert1  [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z      4     I      I      I      I      I     -8      I      I      I      18
   *Dist     0.5   0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi          2.0    0.0   -2.0   -4.0   -6.0          -1.5    5.0    11.5
   * */

  EXPECT_TRUE( equality( verticesInterpolated[2]->z(), -2 ) );
  EXPECT_TRUE( equality( verticesInterpolated[8]->z(), 11.5 ) );

  //change the z value of extremity
  simpleZSpecifierFactory.setZValue( 10 );
  vert1.setZSpecifier( simpleZSpecifierFactory );
  simpleZSpecifierFactory.setZValue( 24 );
  vert3.setZSpecifier( simpleZSpecifierFactory );

  //******************************************
  /*
   *     Vert1  [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z      10    I      I      I      I      I     -8      I      I      I     24
   *Dist     0.5   0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi          7.0    4.0    1.0  -2.0    -5.0           0.0    8.0   16.0
   * */


  EXPECT_TRUE( equality( verticesInterpolated[2]->z(), 1 ) );
  EXPECT_TRUE( equality( verticesInterpolated[8]->z(), 16 ) );


  //remove one extremity
  vert1.hasToBeRemoved();
  simpleZSpecifierFactory.setZValue( 7 );
  //vert3.setZSpecifier( simpleZSpecifierFactory );
  verticesInterpolated[5]->setZSpecifier( simpleZSpecifierFactory );

  //******************************************
  /*
   *            [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z           7.0     I      I      I      I      7      I      I      I      24
   *Dist           0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi                 7.0    7.0    7.0    7.0          11.25  15.5   19.75
   * */

  EXPECT_TRUE( equality( verticesInterpolated[8]->z(), 19.75 ) ) ;
  EXPECT_TRUE( equality( verticesInterpolated[2]->z(), 7 ) );

  //compatibility with combinaison with new gapAndVertexZSpecifier
  gapZSpecifierFactory.setOtherVertex( verticesInterpolated[2] );
  EXPECT_FALSE( verticesInterpolated[3]->isSpecifierIsCompatible( gapZSpecifierFactory ) );
  EXPECT_FALSE( verticesInterpolated[4]->isSpecifierIsCompatible( gapZSpecifierFactory ) );
  EXPECT_FALSE( verticesInterpolated[5]->isSpecifierIsCompatible( gapZSpecifierFactory ) ); //is not a interpolated point anymore
  EXPECT_TRUE( verticesInterpolated[6]->isSpecifierIsCompatible( gapZSpecifierFactory ) ); //a interpolated point that doesn't depend from the sames vertices ast the [2]
  EXPECT_TRUE( vert.isSpecifierIsCompatible( gapZSpecifierFactory ) );

  //remove one interplated point
  verticesInterpolated[7]->hasToBeRemoved();
  delete verticesInterpolated[7];
  simpleZSpecifierFactory.setZValue( 7 );
  vert3.setZSpecifier( simpleZSpecifierFactory );
  //******************************************
  /*
   *            [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z           7.0     I      I      I      I      7    11.25         19.75     7
   *Dist           0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi                 7.0    7.0    7.0    7.0            7             7
   * */


  EXPECT_TRUE( equality( verticesInterpolated[8]->z(), 19.75 ) );

  verticesInterpolated[6]->hasToBeRemoved();
  delete verticesInterpolated[6];
  //******************************************
  /*
   *            [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]    [8]    vert3
   *Z           7.0     I      I      I      I      7                  19.75     7
   *Dist           0.5    0.5    0.5    0.5    0.5    0.5    0.5    0.5     0.5
   *Zi                 7.0    7.0    7.0    7.0                          7
   * */

  EXPECT_TRUE( equality( verticesInterpolated[8]->z(), 19.75 ) );


}

TEST_F( VertexZSpecifierTesting, InterpolateVertices_compatibility )
{

  vert1.setZValue( 2 );
  slopeZSpecifierFactory.setOtherVertex( &vert1 );
  slopeZSpecifierFactory.setSlope( 0.01 );
  vert2.setZSpecifier( slopeZSpecifierFactory );

  interpolationZSpecifierFactory.setExtremitiesVertices( &vert1, &vert2 );
  EXPECT_TRUE( vert4.isSpecifierIsCompatible( interpolationZSpecifierFactory ) );

  vert4.setZSpecifier( interpolationZSpecifierFactory );
  EXPECT_TRUE( equality( vert4.z(), 2.025 ) );

  interpolationZSpecifierFactory.setExtremitiesVertices( &vert1, &vert4 );
  EXPECT_TRUE( vert3.isSpecifierIsCompatible( interpolationZSpecifierFactory ) );
  vert3.setZSpecifier( interpolationZSpecifierFactory );

  vert1.hasToBeRemoved();
  vert2.hasToBeRemoved();
  EXPECT_TRUE( equality( vert4.z(), 2.025 ) );

}


TEST_F( VertexZSpecifierTesting, returningData )
{
  ReosVertexZSpecifier::Data data = vert1.zSpecifier()->data();

  EXPECT_THAT( data.type, Eq( "simple" ) );
  ASSERT_THAT( data.doubleData.size(), Eq( 1 ) );
  EXPECT_THAT( data.doubleData[0], Eq( 5 ) );

  slopeZSpecifierFactory.setSlope( 0.1 );
  slopeZSpecifierFactory.setOtherVertex( &vert2 );
  vert1.setZSpecifier( slopeZSpecifierFactory );
  data = vert1.zSpecifier()->data();

  EXPECT_THAT( data.type, Eq( "vertexSlope" ) );
  ASSERT_THAT( data.doubleData.size(), Eq( 1 ) );
  EXPECT_THAT( data.doubleData[0], Eq( 0.1 ) );
  ASSERT_THAT( data.otherVertices.size(), Eq( 1 ) );
  EXPECT_THAT( data.otherVertices[0], Eq( &vert2 ) );

  gapZSpecifierFactory.setGap( 0.5 );
  gapZSpecifierFactory.setOtherVertex( &vert3 );
  vert1.setZSpecifier( gapZSpecifierFactory );
  data = vert1.zSpecifier()->data();

  EXPECT_THAT( data.type, Eq( "vertexGap" ) );
  ASSERT_THAT( data.doubleData.size(), Eq( 1 ) );
  EXPECT_THAT( data.doubleData[0], Eq( 0.5 ) );
  ASSERT_THAT( data.otherVertices.size(), Eq( 1 ) );
  EXPECT_THAT( data.otherVertices[0], Eq( &vert3 ) );

  vert1.setZSpecifier( simpleZSpecifierFactory );
  interpolationZSpecifierFactory.setExtremitiesVertices( &vert1, &vert4 );
  vert2.setZSpecifier( interpolationZSpecifierFactory );
  vert3.setZSpecifier( interpolationZSpecifierFactory );
  data = vert2.zSpecifier()->data();

  EXPECT_THAT( data.type, Eq( "interpolator" ) );
  ASSERT_THAT( data.doubleData.size(), Eq( 0 ) );
  ASSERT_THAT( data.otherVertices.size(), Eq( 4 ) );
  EXPECT_THAT( data.otherVertices[0], Eq( &vert1 ) );
  EXPECT_THAT( data.otherVertices[1], Eq( &vert4 ) );
  EXPECT_THAT( data.otherVertices[2], Eq( nullptr ) );
  EXPECT_THAT( data.otherVertices[3], Eq( &vert3 ) );

  data = vert3.zSpecifier()->data();

  EXPECT_THAT( data.type, Eq( "interpolator" ) );
  ASSERT_THAT( data.doubleData.size(), Eq( 0 ) );
  ASSERT_THAT( data.otherVertices.size(), Eq( 4 ) );
  EXPECT_THAT( data.otherVertices[0], Eq( &vert1 ) );
  EXPECT_THAT( data.otherVertices[1], Eq( &vert4 ) );
  EXPECT_THAT( data.otherVertices[2], Eq( &vert2 ) );
  EXPECT_THAT( data.otherVertices[3], Eq( nullptr ) );


}





