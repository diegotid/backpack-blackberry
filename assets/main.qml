import bb.cascades 1.0

TabbedPane {
    id: mainPage
    showTabsOnActionBar: true
    
    function reloadBackgrounds() {
        homePage.loadBackground()
        browsePage.loadBackground()
        putinPage.loadBackground()
    }
    
    attachedObjects: [
        Sheet {
            id: settingsSheet
            SettingsSheet {
                onClose: settingsSheet.close();
            }
        },
        Sheet {
            id: helpSheet
            HelpSheet {
                onClose: helpSheet.close();
            }
        },
        Sheet {
            id: aboutSheet
            AboutSheet {
                onClose: aboutSheet.close();
            }
        },
        Sheet {
            id: invokedSheet
            objectName: "invokedSheet"
            InvokedForm {
                onClose: invokedSheet.close();
            }
        }
    ]
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered: settingsSheet.open();
        }
        helpAction: HelpActionItem {
            onTriggered: helpSheet.open();
        }
        actions: [
            ActionItem {
                title: "About"
                imageSource: "asset:///images/menuicons/ic_info.png"
                onTriggered: aboutSheet.open();
            }
        ]
    }
    
    Tab {
        id: readTab
        objectName: "readTab"
        
        title: "Read"
        imageSource: "asset:///images/menuicons/ic_doctype_web.png"

        HomePage {
            id: homePage
        }
    }
    
    Tab {
        id: exploreTab
        objectName: "exploreTab"
        
        title: "Explore"
        imageSource: "asset:///images/menuicons/ic_search.png"

        BrowsePage {
            id: browsePage
        }
    }
    
    Tab {
        id: putinTab
        
        title: "Put in"
        imageSource: "asset:///images/menuicons/ic_doctype_add.png"

		PutinPage {
      		id: putinPage
        }
    }
}
