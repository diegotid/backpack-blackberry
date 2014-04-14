
import bb.cascades 1.0

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
        pocketSync.visible = (username == "")
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
            
            Label {
                id: pocketHintA
                multiline: true	            
                text: "You can also sync your Pocket account..."
                onVisibleChanged: pocketHintB.visible = !pocketHintA.visible
            }
            
            Label {
                id: pocketHintB
                multiline: true	            
                text: "You can also add content from Pocket"
                visible: false
            }
            
            ImageView {
                imageSource: "asset:///images/empty-hint.png"
                horizontalAlignment: HorizontalAlignment.Center
                scalingMethod: ScalingMethod.AspectFit
                visible: pageHandler.layoutFrame.height > 720 || freeTitleBar.username.length > 0
            }
        }
        
        Container {
            horizontalAlignment: HorizontalAlignment.Center
            rightPadding: 42
            leftPadding: 42
            bottomPadding: 45
            
            TextField {
                hintText: "Search something to put in..."
                input.submitKey: SubmitKey.Search
                input.onSubmitted: app.launchSearchToPutin(text)
            }
        }
        
        Container {
            id: pocketSync
            rightPadding: 42
            leftPadding: 42
            horizontalAlignment: HorizontalAlignment.Fill
            onVisibleChanged: pocketHintA.visible = pocketSync.visible
            
            Button {
                text: "Sync with Pocket"
                horizontalAlignment: HorizontalAlignment.Fill
                bottomMargin: 3
                onClicked: {
                    pocketPage.state = "sync"
                    pocketSheet.open()
                }
            }
            
            Label {
                text: "<span style='text-decoration:underline'>Why Pocket?</span>"
                textFormat: TextFormat.Html
                horizontalAlignment: HorizontalAlignment.Right
                textStyle.fontSize: FontSize.Small
                onTouch: {
                    pocketPage.state = "why"
                    pocketSheet.open()
                }
            }            
        }
    }	        
}
