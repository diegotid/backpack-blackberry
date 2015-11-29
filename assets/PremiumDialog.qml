
import bb.cascades 1.4

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
                maxWidth: ui.sdu(70)
                topPadding: ui.sdu(3)
                bottomPadding: ui.sdu(3)
                leftPadding: ui.sdu(3)
                rightPadding: ui.sdu(3)
                
                Label {
                    id: dialogTitle
                    textStyle.color: Color.create("#323232")
                    bottomMargin: ui.sdu(2.5)
                }
                
                Container {
                    id: confirmation
                    visible: false
                    bottomMargin: ui.sdu(2)
                    minWidth: dialogHandler.layoutFrame.width
                
                    Label {
                        text: "Features unlocked:"
                        + "\n- Article search"
                        + "\n- Picking of an specific article"
                        + "\n- Article skipping on Lounge & Quickest"
                        multiline: true
                        textStyle.color: Color.DarkGray
                        bottomMargin: ui.sdu(2)
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
                    minWidth: dialogHandler.layoutFrame.width
                    bottomPadding: ui.sdu(3)

                    Label {
                        id: errorLabel
                        multiline: true
                        textStyle.color: Color.DarkGray
                        bottomMargin: ui.sdu(4)
                    }
                    
                    Button {
                        text: "Try again now"
                        horizontalAlignment: HorizontalAlignment.Fill
                        color: Color.White
                        onClicked: premiumDialog.parent.parent.confirmPurchase()
                    }
                }
                
                Container {
                    id: approach
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomPadding: ui.sdu(2)
                    
                    attachedObjects: LayoutUpdateHandler {
                        id: dialogHandler
                    }
                    
                    Label {
                        text: "Some features are only available on the full version of Backpack:"
                        + " Article search, article skipping and article picking"
                        multiline: true
                        textStyle.color: Color.DarkGray
                        bottomMargin: ui.sdu(3)
                    }
                    
                    Button {
                        text: "Get it now!"
                        horizontalAlignment: HorizontalAlignment.Fill
                        color: Color.DarkGreen
                        onClicked: premiumDialog.parent.parent.confirmPurchase()
                        bottomMargin: 0
                    }
                    
                    Label {
                        id: priceLabel
                        text: "One-time single payment"
                        textStyle.color: Color.DarkGray
                        topMargin: ui.sdu(1)
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
                        topMargin: ui.sdu(2)
                    }
                }
                
                Button {
                    id: closeButton
                    horizontalAlignment: HorizontalAlignment.Fill
                    color: Color.White
                    onClicked: premiumDialog.close()
                }            
            }
        }
    }
}
