
import bb.cascades 1.0

Page {
    
    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            FreeTitleBar {
                id: freeTitleBar
            }
        }
        scrollBehavior: TitleBarScrollBehavior.Sticky // Comment for 10.0
    }
    
    function updateUsername(username) {
        freeTitleBar.username = username
        pocketSync.visible = (username == "")
    }
    
    onCreationCompleted: loadBackground()
    
    function loadBackground() {
        backgroundRed.opacity = app.getBackgroundColour("red");
        backgroundGreen.opacity = app.getBackgroundColour("green");
        backgroundBlue.opacity = app.getBackgroundColour("blue");
        backgroundBase.opacity = app.getBackgroundColour("base");
    }
    
    attachedObjects: [
        ImagePaintDefinition {
            id: background
            imageSource: "asset:///images/shadow.png"
            repeatPattern: RepeatPattern.X
        },
        Sheet {
            id: pocketSheet
            PocketSheet {
                id: pocketPage
                onClose: pocketSheet.close()
            }
        }
    ]
    
    function pocketState() {
        pocketPage.state = "on"
        pocketSheet.open()
    }
    
    Container {
        layout: AbsoluteLayout {}
        
        ImageView {
            id: backgroundRed
            imageSource: "asset:///images/background-red.png"
        }
        ImageView {
            id: backgroundGreen
            imageSource: "asset:///images/background-green.png"
        }
        ImageView {
            id: backgroundBlue
            imageSource: "asset:///images/background-blue.png"
        }
        ImageView {
            id: backgroundBase
            imageSource: "asset:///images/background.png"
        }
        
        Container {
            
            Container {
                topPadding: 30
                leftPadding: 30
                rightPadding: 30
                bottomPadding: 30
                background: background.imagePaint
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
                }
            }
            
            Container {
                horizontalAlignment: HorizontalAlignment.Center
                rightPadding: 60
                leftPadding: 60
                bottomPadding: 45
                
                TextField {
                    hintText: "Search something to put in..."
                    input.submitKey: SubmitKey.Search
                    input.onSubmitted: app.launchSearchToPutin(text)
                }
            }
            
            Container {
                id: pocketSync
                rightPadding: 50
                leftPadding: 50
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
}
