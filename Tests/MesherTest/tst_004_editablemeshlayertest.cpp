
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <qgsproviderregistry.h>

#include "../../Mesher/provider/meshdataprovider.h"


using namespace testing;

class TinLayerTesting: public Test
{
  public:
    QgsProviderRegistry *providerRegistery = QgsProviderRegistry::instance();
    QgsMeshLayer *layer = nullptr;

    void initializeLayerWithOneVertex()
    {
      layer = new QgsMeshLayer( "oneVertexTIN", "nom", "TIN" );
      TINProvider *provider = static_cast<TINProvider *>( layer->dataProvider() );
      provider->tin()->addVertex( 0, 0 );
    }

    void initializeLayerWithSeveralVertices()
    {
      layer = new HdTinLayer( "SeveralVerticesTIN" );
      TINProvider *provider = static_cast<TINProvider *>( layer->dataProvider() );

      provider->tin()->addVertex( 0, 0 );
      provider->tin()->addVertex( 0, 5 );
      provider->tin()->addVertex( 5, 5 );
      provider->tin()->addVertex( 5, 0 );
      provider->tin()->addVertex( 0, 5 );
    }

    void registerProvider()
    {
      providerRegistery->registerProvider( new HdTinEditorProviderMetaData() );
    }



    // Test interface
  protected:
    void SetUp() override
    {
      providerRegistery->registerProvider( new HdTinEditorProviderMetaData() );
    }
};

TEST_F( TinLayerTesting, registerProvider )
{
  registerProvider();
  ASSERT_TRUE( providerRegistery->providerList().contains( "TIN" ) );
}

TEST_F( TinLayerTesting, createProvider )
{
  registerProvider();
  QgsDataProvider *provider = providerRegistery->createProvider( "TIN", "" );
  ASSERT_TRUE( provider->isValid() );
}

TEST_F( TinLayerTesting, layerHasProvider )
{
  registerProvider();
  HdTinLayer *layer = new HdTinLayer( "-" );

  ASSERT_TRUE( layer->dataProvider() != nullptr );
}

TEST_F( TinLayerTesting, layerIsValid )
{
  registerProvider();
  HdTinLayer *layer = new HdTinLayer( "-" );

  ASSERT_TRUE( layer->isValid() );
}



TEST_F( TinLayerTesting, verticesCount )
{
  registerProvider();
  initializeLayerWithSeveralVertices();

  ASSERT_THAT( layer->dataProvider()->vertexCount(), Eq( 4 ) );
}


TEST_F( TinLayerTesting, facesCount )
{
  registerProvider();
  initializeLayerWithSeveralVertices();

  ASSERT_THAT( layer->dataProvider()->faceCount(), Eq( 2 ) );
}


TEST_F( TinLayerTesting, populateMesh )
{
  registerProvider();
  initializeLayerWithSeveralVertices();

  QgsMesh mesh;

  layer->dataProvider()->populateMesh( &mesh );

  ASSERT_THAT( mesh.faceCount(), Eq( 2 ) );
  ASSERT_THAT( mesh.vertexCount(), Eq( 4 ) );
}


TEST_F( TinLayerTesting, extentMesh )
{
  registerProvider();
  initializeLayerWithSeveralVertices();


  qDebug() << layer->dataProvider()->extent().xMinimum();
  qDebug() << layer->dataProvider()->extent().yMinimum();
  qDebug() << layer->dataProvider()->extent().xMaximum();
  qDebug() << layer->dataProvider()->extent().yMaximum();

  QgsRectangle extent = layer->dataProvider()->extent();

  ASSERT_TRUE( extent == QgsRectangle( 0, 0, 5, 5 ) );

}

TEST_F( TinLayerTesting, save )
{
  registerProvider();
  initializeLayerWithSeveralVertices();


  ReosTin *tin = static_cast<TINProvider *>( layer->dataProvider() )->tin();
  ASSERT_THAT( tin->writeUGRIDFormat( "tinTestEditor.tin" ), Eq( 0 ) );

}

TEST_F( TinLayerTesting, open )
{
  registerProvider();

  QgsMeshLayer *layerToRead = new QgsMeshLayer( "tinTestEditor.tin", "nom", "TIN" );

  ASSERT_TRUE( layerToRead->isValid() );
  ASSERT_THAT( layerToRead->dataProvider()->vertexCount(), Eq( 4 ) );
  ASSERT_THAT( layerToRead->dataProvider()->faceCount(), Eq( 2 ) );
}

