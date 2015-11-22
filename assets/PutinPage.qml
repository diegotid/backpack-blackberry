
import bb.cascades 1.3

Page {

    actionBarVisibility: ChromeVisibility.Overlay
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll

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
    
    function updateProgress(progress) {
        freeTitleBar.progress = progress
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
        pocketPage.username = freeTitleBar.username
        pocketPage.state = "why"
        pocketSheet.open()
    }
    
    actions: [
        ActionItem {
            title: "Pocket sync"
            imageSource: "asset:///images/menuicons/pocket.png"
            ActionBar.placement: ActionBarPlacement.Signature
            onTriggered: {
                pocketPage.state = "sync"
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
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                horizontalAlignment: HorizontalAlignment.Center
                topMargin: 20
                ImageView {
                    imageSource: "asset:///images/menuicons/ic_overflow_action.png"
                }
                ImageView {
                    imageSource: "asset:///images/next_step.png"
                    rightMargin: 15
                }
                ImageView {
                    imageSource: "asset:///images/menuicons/ic_share.png"
                }
                ImageView {
                    imageSource: "asset:///images/next_step.png"                                
                    rightMargin: 15
                }
                ImageView {
                    imageSource: "asset:///images/share-sample.png"
                    verticalAlignment: VerticalAlignment.Center
                }
            }
            
            TextField {
                hintText: "Search something to put..."
                input.submitKey: SubmitKey.Search
                input.onSubmitted: app.launchSearchToPutin(text)
                topMargin: ui.du(3)
                bottomMargin: ui.du(3)
            }
            
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
