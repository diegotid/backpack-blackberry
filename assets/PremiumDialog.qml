
import bb.cascades 1.2

Dialog {
    id: premiumDialog
    
    property string price
    onPriceChanged: {
        if (priceLabel.text.indexOf(":") < 0) {
            priceLabel.text += ": " + price
        }
    }
    
    function initiate() {
        dialogTitle.text = "Get Backpack full version"
        closeButton.text = "I'll settle for free version"
        confirmation.visible = false
        errorMessage.visible = false
        approach.visible = true
    }
    
    function confirmPurchase() {
        dialogTitle.text = "Purchase completed!"
        closeButton.text = "Check it!"
        confirmation.visible = true
        errorMessage.visible = false
        approach.visible = false
    }
    
    function reportError(errorText) {
        errorMessage.visible = true
        approach.visible = false
        errorLabel.text = "Purchase didn't complete: " + errorText
        closeButton.text = "Try later"
    }
    
    Container {
        layout: DockLayout {}
        verticalAlignment: VerticalAlignment.Fill
        horizontalAlignment: HorizontalAlignment.Fill
        background: Color.create(0.0, 0.0, 0.0, 0.5)
        
        ScrollView {
            verticalAlignment: VerticalAlignment.Center
            horizontalAlignment: HorizontalAlignment.Center
        
            Container {
                background: Color.White
                maxWidth: 700
                topPadding: 30
                bottomPadding: 30
                leftPadding: 30
                rightPadding: 30
                
                Label {
                    id: dialogTitle
                    textStyle.color: Color.create("#323232")
                    bottomMargin: 25
                }
                
                Container {
                    id: confirmation
                    visible: false
                    bottomMargin: 20
                
                    Label {
                        text: "Features unlocked:"
                        + "\n- Article search"
                        + "\n- Picking of an specific article"
                        + "\n- Article skipping on Lounge & Quickest"
                        multiline: true
                        textStyle.color: Color.DarkGray
                        bottomMargin: 20
                    }
                    
                    ImageView {
                        imageSource: "asset:///images/premium.png"
                        horizontalAlignment: HorizontalAlignment.Center
                    }
                }
                
                Container {
                    id: errorMessage
                    visible: false
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomPadding: 30

                    Label {
                        id: errorLabel
                        multiline: true
                        textStyle.color: Color.DarkGray
                        bottomMargin: 40
                    }
                    
                    Button {
                        text: "Try again now"
                        horizontalAlignment: HorizontalAlignment.Fill
                        onClicked: premiumDialog.parent.parent.confirmPurchase()
                    }
                }
                
                Container {
                    id: approach
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomPadding: 20
                    
                    Label {
                        text: "Some features are only available on the full version of Backpack:"
                        + " Article search, article skipping and article picking"
                        multiline: true
                        textStyle.color: Color.DarkGray
                        bottomMargin: 30
                    }
                    
                    Button {
                        text: "Get it now!"
                        horizontalAlignment: HorizontalAlignment.Fill
                        onClicked: premiumDialog.parent.parent.confirmPurchase()
                        bottomMargin: 0
                    }
                    
                    Label {
                        id: priceLabel
                        text: "One-time single payment"
                        textStyle.color: Color.DarkGray
                        topMargin: 10
                    }
                    
                    Label {
                        text: "Basic features are still free: Standalone or Pocket synced,"
                        + " Offline reading of Shuffle, Lounge or Quickest articles"
                        multiline: true
                        textStyle.color: Color.DarkGray
                    }
                    
                    ImageView {
                        imageSource: "asset:///images/freemium.png"
                        horizontalAlignment: HorizontalAlignment.Center
                        topMargin: 20
                    }
                }
                
                Button {
                    id: closeButton
                    horizontalAlignment: HorizontalAlignment.Fill
                    onClicked: premiumDialog.close()
                }            
            }
        }
    }
}
