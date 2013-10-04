import bb.cascades 1.0

TabbedPane {
    id: mainPage
    showTabsOnActionBar: true
    
    function reloadBackgrounds() {
        explorePage.reloadBackgrounds()
        addinPage.reloadBackgrounds()
    }
    
    attachedObjects: [
        Sheet {
            id: settingsSheet
            SettingsSheet {
                onClose: settingsSheet.close();
            }
        }
    ]
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered: settingsSheet.open();
        }
    }
    
    Tab {
        id: readTab
        objectName: "readTab"
        
        title: "Read"
        imageSource: "asset:///images/menuicons/ic_doctype_web.png"

        HomePage {}
    }
    
    Tab {
        id: exploreTab
        objectName: "exploreTab"
        
        title: "Explore"
        imageSource: "asset:///images/menuicons/ic_search.png"

        BrowsePage {
            id: explorePage
        }
    }
    
    Tab {
        id: addinTab
        
        title: "Put in"
        imageSource: "asset:///images/menuicons/ic_doctype_add.png"

		PutinPage {
      		id: putinPage
        }
    }
}
