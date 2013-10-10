
import bb.cascades 1.0

Page {
    
    titleBar: TitleBar {
        title: "My Backpack"
//        scrollBehavior: TitleBarScrollBehavior.Sticky // Comment for 10.0
    }
    
    attachedObjects: [
        ImagePaintDefinition {
            id: background
            imageSource: "asset:///images/shadow.png"
            repeatPattern: RepeatPattern.X
        }
    ]
    
    onCreationCompleted: loadBackground()
    
    function loadBackground() {
        backgroundRed.opacity = app.getBackgroundColour("red");
        backgroundGreen.opacity = app.getBackgroundColour("green");
        backgroundBlue.opacity = app.getBackgroundColour("blue");
        backgroundBase.opacity = app.getBackgroundColour("base");
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
            layout: DockLayout {}
            
            Container {
                topPadding: 30
                bottomPadding: 120
                leftPadding: 30
                rightPadding: 30
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
                    text: "To put content in your backpack just use the share menu option from your browser on any page and select \"My Backpack\""
                }
                
                ImageView {
                    imageSource: "asset:///images/empty-hint.png"
                    horizontalAlignment: HorizontalAlignment.Center
                    scalingMethod: ScalingMethod.AspectFit
                    topMargin: 30
                }
            }
            
            Container {
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment: VerticalAlignment.Bottom
                rightPadding: 50
                leftPadding: 50
                topPadding: 45
                bottomPadding: 45
                
                TextField {
                    hintText: "Search something to put in..."
                    input.submitKey: SubmitKey.Search
                    input.onSubmitted: app.launchSearchToPutin(text)
                }            
            }
        }	        
	}
}
