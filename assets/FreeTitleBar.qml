import bb.cascades 1.0

Container {
    layout: DockLayout {}
    leftPadding: 20
    rightPadding: 20
    
    property string username
    onUsernameChanged: {
        pocketUsername = username
        pocketSession.visible = username.length > 0
    }
    
    Label {
        text: "Backpack"
        verticalAlignment: VerticalAlignment.Center
        textStyle.fontSize: FontSize.Large
    }
    
    Container {
        id: pocketSession
        visible: username.length > 0
        
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        topPadding: 18
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Right
        
        onTouch: putinPage.pocketState()
        
        ImageView {
            imageSource: "asset:///images/menuicons/pocket.png"
            maxWidth: 40
            scalingMethod: ScalingMethod.AspectFit
            translationY: -2
        }
        
        Label {
            id: pocketUsername
            text: username
            leftMargin: 12
        }
    }
}
