
import bb.cascades 1.4

Page {
    
    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            FreeTitleBar {
                id: freeTitleBar
            }
        }
    }
    
    function updateUsername(username) {
        freeTitleBar.username = username
    }
    
    attachedObjects: [
        Sheet {
            id: pocketSheet
            PocketSheet {
                id: pocketPage
                onClose: pocketSheet.close()
            }
        }
    ]
    
    function pocketState() {
        pocketPage.state = freeTitleBar.username.length > 0 ? "on" : "why"
        pocketSheet.open()
    }
    
    actions: [
        ActionItem {
            title: "Pocket sync"
            imageSource: "asset:///images/menuicons/pocket.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                pocketPage.state = (username == "") ? "sync" : "on"
                pocketSheet.open()
            }
        },
        ActionItem {
            title: "Why Pocket?"
            enabled: (username == "")
            imageSource: "asset:///images/menuicons/ic_info.png"
            onTriggered: {
                pocketPage.state = "why"
                pocketSheet.open()
            }
        }
    ]
    
    Container {
        
        attachedObjects: LayoutUpdateHandler {
            id: pageHandler
        }
        
        Container {
            topPadding: 30
            leftPadding: 30
            rightPadding: 30
            bottomPadding: 30
            horizontalAlignment: HorizontalAlignment.Fill
            
            Label {
                id: emptyHint
                objectName: "emptyHint"
                text: "Nothing in your backpack"
                visible: false
            }
            
            Label {
                multiline: true	            
                text: "To put content in your backpack just use the share menu option from your browser on any page and select \"Backpack\""
            }
            
            ImageView {
                imageSource: "asset:///images/empty-hint.png"
                horizontalAlignment: HorizontalAlignment.Center
                scalingMethod: ScalingMethod.AspectFit
                visible: pageHandler.layoutFrame.height > 720 || freeTitleBar.username.length > 0
            }
            
            TextField {
                hintText: "Search something to put in..."
                input.submitKey: SubmitKey.Search
                input.onSubmitted: app.launchSearchToPutin(text)
                topMargin: ui.du(3)
                bottomMargin: ui.du(5)
            }

            Divider {}
            
            Label {
                id: pocketHint
                multiline: true	            
                text: "You can also sync your Pocket account in order to get in your Backpack everything you add to it..."
                visible: (username == "")
            }
            
            Label {
                multiline: true	            
                text: "You can also add content from your Pocket account"
                visible: !pocketHint.visible
            }
        }
    }	        
}
