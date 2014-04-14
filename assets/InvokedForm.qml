
import bb.cascades 1.0

Page {
    id: invokedForm
    objectName: "invokedForm"
    
    property variant item
    signal close();
    
    titleBar: TitleBar {
        title: "Backpack"
        
        dismissAction: ActionItem {
            id: dismissButton
            objectName: "dismissButton"
            title: "Close"
            
            onTriggered: {
                invokedImage.resetImage()
                invokedFavicon.resetImage()
                if (invokedForm.parent.objectName == "bookmarkSheet") {
                    invokedForm.close()
                } else {
                    Application.quit()
                }
            }
        }
        
        acceptAction: ActionItem {
            id: acceptButton
            objectName: "acceptButton"
            title: "Save"
            enabled: false
            
            onTriggered: {
                item ? app.memoBookmark(item.url, memo.text) : app.memoBookmark(memo.text)
                invokedForm.close()
            }
        }
    }
    
    paneProperties: NavigationPaneProperties {
        backButton: dismissButton
    }
    
    Container {
        layout: DockLayout {}
        
        attachedObjects: LayoutUpdateHandler {
            id: pageHandler
        }
        
        Container {
            layout: AbsoluteLayout {}
            verticalAlignment: VerticalAlignment.Bottom
            
            ImageView {
                id: invokedImage
                objectName: "invokedImage"
                imageSource: item ? item.image : ""
                scalingMethod: ScalingMethod.AspectFill
                verticalAlignment: VerticalAlignment.Fill
                horizontalAlignment: HorizontalAlignment.Fill
                minWidth: pageHandler.layoutFrame.width
                preferredHeight: pageHandler.layoutFrame.height
                opacity: 0.5
                
                attachedObjects: LayoutUpdateHandler {
                    id: imageHandler
                }
            }
            
            ImageView {
                imageSource: "asset:///images/home-shadow.png"
                scalingMethod: ScalingMethod.AspectFill
                verticalAlignment: VerticalAlignment.Fill
                horizontalAlignment: HorizontalAlignment.Fill
                minWidth: imageHandler.layoutFrame.width
                minHeight: imageHandler.layoutFrame.height
                rotationZ: 180.0
            }
        }

        Container { // Status
            topPadding: 25
            leftPadding: 25
            rightPadding: 25
            bottomPadding: 25
            
            Label {
                id: status
                objectName: "status"
                textStyle.color: Color.create("#07b1e6")
                textStyle.fontSize: FontSize.Large
                translationX: 8
//                text: "Comment this for release"
            }			
            
            Container { // Activity & title
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                maxHeight: 145
                leftPadding: 5
                rightPadding: 5
                bottomMargin: 0
                
                Container {
                    objectName: "activity"
                    topPadding: 15
                    rightPadding: 15
                    leftPadding: 4
                    visible: !item
//                    visible: false
                    
                    ActivityIndicator {
                        running: true
                    }             
                }
                
                Label {
                    id: title
                    objectName: "title"
                    textStyle.fontSize: FontSize.XLarge
                    textStyle.color: Color.White
                    verticalAlignment: VerticalAlignment.Center
                    multiline: true
                    text: item ? item.title : ""
//                    text: "Comment this for release but must be at least three lines adslfkjasñdlkfjasñld  fñlaskdjf a"
                }
            }
            
            Container { // Memo & keep switch
                leftPadding: 5
                rightPadding: 5
                
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    topPadding: 5
                    bottomPadding: 10

                    ImageView {
                        id: invokedFavicon
                        objectName: "invokedFavicon"
                    	imageSource: item ? item.favicon : "asset:///images/favicon.png"
                    	minHeight: 26
                    	minWidth: 26
                    	translationY: 5
                        visible: invokedURL.text.toString().length > 0
                    }
                    
                    Label {
                        id: invokedURL
                        objectName: "invokedURL"
                        text: item.url
                        textStyle.color: Color.LightGray
                        textStyle.fontSize: FontSize.XSmall
                    }
                }
                
                TextArea {
                    id: memo
                    objectName: "memo"
                    preferredHeight: 185
                    maxHeight: 185
                    
                    text: item ? item.memo : ""
                    hintText: "Type your memo here (optional)"
//                    text: "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum"
                    
                    onTextChanging: {
                        if (focused) { // Don't do this when opened from the edit action
                            acceptButton.enabled = (memo.text != "");
                            dismissButton.title = (memo.text != "") ? "Cancel" : "Close";
                        }
                    }
                }
                
                Container {
                    layout: DockLayout {}
                    topPadding: 10
                    bottomPadding: 35
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        
                        ImageView {
                            id: keepStar
                            imageSource: "asset:///images/menuicons/zip.png"
                            translationX: -15
                            scaleX: 0.9
                            scaleY: 0.9
                            scalingMethod: ScalingMethod.AspectFit
                        }
                        
                        Label {
                            text: "Favorite"
                            verticalAlignment: VerticalAlignment.Center
                            translationX: -25
                        }
                        
                        Label {
                            text: "(keep after read)"
                            visible: !app.getKeepAfterRead()
                            textStyle.color: Color.LightGray
                            verticalAlignment: VerticalAlignment.Center
                            translationX: -25
                            translationY: 3
                            textStyle.fontSize: FontSize.XSmall
                        }
                    }
                    
                    ToggleButton {
                        id: keepCheck
                        objectName: "keepCheck"
                        horizontalAlignment: HorizontalAlignment.Right                  
                        verticalAlignment: VerticalAlignment.Center
                        checked: item && item.keep == "true"
                        property bool invokeChecked: false
                        
                        onCheckedChanged: {
                            item ? app.keepBookmark(item.url, checked) : app.keepBookmark(checked)
                            keepStar.imageSource = "asset:///images/menuicons/zip" + (checked ? "On" : "") + ".png"
                        }
                        onInvokeCheckedChanged: keepCheck.checked = invokeChecked
                    }
                }
            }
        }	        	                   
    }
}
