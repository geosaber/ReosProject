#ifndef TST_006_HDTINEDITORGRAPHICTEST_H
#define TST_006_HDTINEDITORGRAPHICTEST_H


#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <qgsmeshdataprovider.h>
#include <qgsproviderregistry.h>

#include "../../Mesher/hdtineditorgraphic.h"
#include "../../Mesher/meshdataprovider.h"

using namespace testing;

class UIMeshEditorTesting : public Test{
public:
    QgsMapCanvas *mapCanvas;
    HdTINEditorUI *uiEditor;
    QgsMeshLayer *meshLayer;


    bool testUiEditorActionEnable()
    {
        QList<QAction*> actionsList=uiEditor->getActions();
        int active=true;
        for (auto a:actionsList)
            active&=a->isEnabled();

        active|=(actionsList.isEmpty());

        return active;
    }

    // Test interface
protected:
    void SetUp() override
    {
        QgsProviderRegistry::instance()->registerProvider(new HdMeshEditorProviderMetaData());
        mapCanvas=new QgsMapCanvas();
        uiEditor=new HdTINEditorUI(mapCanvas);
        meshLayer=new QgsMeshLayer("-","Mesh editable","TIN");
    }

    void TearDown() override
    {
        delete mapCanvas;
        delete uiEditor;
        delete  meshLayer;
    }

};

TEST_F(UIMeshEditorTesting,actionsAreDisable)
{
    uiEditor->setMeshLayer(nullptr);
    ASSERT_THAT(testUiEditorActionEnable(),Eq(false));
}

TEST_F(UIMeshEditorTesting,actionsAreEnable)
{
    uiEditor->setMeshLayer(meshLayer);
    ASSERT_THAT(testUiEditorActionEnable(),Eq(true));
}

TEST_F(UIMeshEditorTesting,actionsAreDisableAgain)
{
    uiEditor->setMeshLayer(meshLayer);
    uiEditor->setMeshLayer(nullptr);

    ASSERT_THAT(testUiEditorActionEnable(),Eq(false));
}



#endif // TST_006_HDTINEDITORGRAPHICTEST_H