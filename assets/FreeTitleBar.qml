
import bb.cascades 1.3

Container {
    layout: DockLayout {}
    
    property string username
    onUsernameChanged: {
        pocketUsername.text = username
        pocketSession.visible = username.length > 0
    }
    
    property real progress
    onProgressChanged: {
        downloadingProgress.value = progress
        if (progress == 1) {
            downloadingText.fadeOut()
        } else if (progress > 0 && !downloadingText.visible) {
            downloadingText.fadeIn()
        }
    }

    Container {
        ProgressIndicator {
            id: downloadingProgress
            objectName: "downloadingProgress"
            visible: value > 0
            scaleY: 1.5
            topMargin: 0
            bottomMargin: 0
        }
    }
    
    Container {
        layout: DockLayout {}
        leftPadding: 6
        rightPadding: 8
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Center

        ImageView {
            imageSource: "asset:///images/title.png"
            horizontalAlignment: HorizontalAlignment.Left
            verticalAlignment: VerticalAlignment.Bottom
            visible: !downloadingProgress.visible
            scaleX: 0.9
            scaleY: 0.9
            translationX: -5
            translationY: 2
        }
        
        Container {
            topPadding: ui.sdu(0.6)
            leftPadding: ui.sdu(0.6)
            Label {
                id: downloadingText
                text: "Downloading... " + parseInt(downloadingProgress.value * 100) + "%"
                textStyle.fontSize: FontSize.Small
                textStyle.color: Color.LightGray
                visible: false
                opacity: 0
                animations: [
                    FadeTransition {
                        id: fadeInAnimation
                        fromOpacity: 0
                        toOpacity: 1
                        duration: 500
                        onStarted: {
                            downloadingText.visible = true
                            downloadingProgress.opacity = 1
                            downloadingProgress.visible = true
                        }
                    },
                    FadeTransition {
                        id: fadeOutAnimation
                        fromOpacity: 1
                        toOpacity: 0
                        duration: 2000
                        onStarted: downloadingProgress.opacity = 0
                        onEnded: {
                            downloadingText.visible = false
                            downloadingProgress.visible = false
                        }
                    }
                ]
                function fadeIn() {
                    fadeInAnimation.play()
                }
                function fadeOut() {
                    fadeOutAnimation.play()
                }
            }
        }
        
        Container {
            id: pocketSession
            visible: username.length > 0
            
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            topPadding: 3
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Right
            
            attachedObjects: LayoutUpdateHandler {
                id: pocketHandler
            }
            
            onTouch: {
                if (event.touchType == TouchType.Down) {
                    putinPage.pocketState()
                }
            }
            
            Container {
                id: pocketConnSignal
                objectName: "pocketConnSignal"
                topPadding: 3
                verticalAlignment: VerticalAlignment.Center
                ImageView {
                    imageSource: "asset:///images/pocket-logo.png"
                }
                visible: !pocketErrorSignal.visible && !syncingActivity.running
            }
            
            Container {
                id: pocketErrorSignal
                objectName: "pocketErrorSignal"
                topPadding: 3
                verticalAlignment: VerticalAlignment.Center
                ImageView {
                    imageSource: "asset:///images/pocket-error.png"
                }
                visible: !pocketConnSignal.visible && !syncingActivity.running
            }
            
            Container {
                topPadding: 1
                verticalAlignment: VerticalAlignment.Center
                ActivityIndicator {
                    id: syncingActivity
                    objectName: "syncingActivity"
                    running: !pocketConnSignal.visible && !pocketErrorSignal.visible
                }
            }
            
            Container {
                topPadding: 2
                leftMargin: 12
                verticalAlignment: VerticalAlignment.Center
                Label {
                    id: pocketUsername
                    text: username
                    textStyle.color: Color.White
                    maxWidth: 320
                }
            }
        }
    }
}