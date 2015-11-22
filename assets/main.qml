
import bb.cascades 1.2

TabbedPane {
    id: mainPage
    
    property string username
    onUsernameChanged: {
        browsePage.updateUsername(username)
        putinPage.updateUsername(username)
    }
    
    property real progress
    onProgressChanged: {
        browsePage.updateProgress(progress)
        putinPage.updateProgress(progress)
    }
    
    attachedObjects: [
        Sheet {
            id: settingsSheet
            SettingsSheet {
                onClose: settingsSheet.close();
            }
        },
        Sheet {
            id: backupSheet
            objectName: "backupSheet"
            BackupSheet {
                objectName: "backupPage"
                onClose: backupSheet.close();
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
                onTriggered: {
                    aboutSheet.open()
                }
            },
            ActionItem {
                title: "Backup"
                imageSource: "asset:///images/menuicons/ic_save.png"
                onTriggered: {
                	backupSheet.open()
                	app.showBackups()
                }
            },
            ActionItem {
                title: "Pocket"
                imageSource: "asset:///images/menuicons/pocket.png"
                onTriggered: putinPage.pocketState()
            }
        ]
    }
    
    Tab {
        id: exploreTab
        objectName: "exploreTab"
        
        title: "Browse"
        imageSource: "asset:///images/menuicons/backpack.png"

        BrowsePage {
            id: browsePage
        }
    }
    
    Tab {
        id: putinTab
        objectName: "putinTab"
        
        title: "Put something in"
        imageSource: "asset:///images/menuicons/ic_add_entry.png"
        
        PutinPage {
      		id: putinPage
        }
    }
}
