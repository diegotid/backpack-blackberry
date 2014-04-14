import bb.cascades 1.0

Container {
    layout: DockLayout {}
    leftPadding: 20
    rightPadding: 20
    
    property string username
    onUsernameChanged: {
        pocketUsername.text = username
        pocketSession.visible = username.length > 0
    }
    
    ImageView {
        imageSource: "asset:///images/title.png"
        verticalAlignment: VerticalAlignment.Center
        scaleX: 0.9
        scaleY: 0.9
        translationX: -5
        translationY: 2
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
        
        onTouch: putinPage.pocketState()
        
        Container {
            id: pocketConnSignal
            objectName: "pocketConnSignal"
            topPadding: 3
            ImageView {
                imageSource: "asset:///images/pocket-logo.png"
            }
            visible: !pocketErrorSignal.visible && !syncingActivity.running
        }
        
        Container {
            id: pocketErrorSignal
            objectName: "pocketErrorSignal"
            topPadding: 3
            ImageView {
                imageSource: "asset:///images/pocket-error.png"
            }
            visible: !pocketConnSignal.visible && !syncingActivity.running
        }
        
        Container {
            topPadding: 1
	        ActivityIndicator {
	            id: syncingActivity
	            objectName: "syncingActivity"
	            running: !pocketConnSignal.visible && !pocketErrorSignal.visible
	        }
        }
        
        Container {
            topPadding: 2
            leftMargin: 12
            Label {
                id: pocketUsername
                text: username
                textStyle.color: Color.White
                maxWidth: 320
            }
        }
    }
}
