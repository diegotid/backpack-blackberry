
import bb.cascades 1.3

Page {
    id: invokedForm
    objectName: "invokedForm"
    
    property variant item
    signal close()
    
    onItemChanged: {
        titleBar.title = "Edit item"
        status.text = ""
        invokedImage.visible = item.image && item.image.toString().length > 0
        invokedFavicon.visible = item.favicon && item.favicon.toString().length > 0
    }
    
    titleBar: TitleBar {
        title: "New item"
        
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
                app.memoBookmark(invokedURL.text, memo.text)
                invokedForm.close()
            }
        }
    }
    
    paneProperties: NavigationPaneProperties {
        backButton: dismissButton
    }
        
    function domain(url) {
        var domain = url.substring(url.indexOf("://") + (url.indexOf("://") < 0 ? 1 : 3));
        console.log("domain: " + domain)
        if (domain.indexOf('/') < domain.indexOf('.')) {
            return domain
        } else {
            return domain.substring(0, domain.indexOf('/'))
        }
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
                imageSource: item ? "file://" + item.image : "asset:///images/backpack.png"
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
            }
        }

        Container { // Status
            topPadding: 25
            leftPadding: 25
            rightPadding: 25
            bottomPadding: 25
            verticalAlignment: VerticalAlignment.Bottom
            
            Label {
                id: status
                objectName: "status"
                textStyle.color: ui.palette.primary
                textStyle.fontSize: FontSize.Large
                translationX: 8
//                text: "Comment this for release"
            }			
            
            Container { // Activity & title
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
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
                        accessibility.name: "Loading content"
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
//                    text: "Título corto de dos líneas por lo menos"
                }
            }
            
            Container { // Memo & keep switch
                leftPadding: 5
                rightPadding: 5
                
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    topPadding: 10
                    bottomPadding: 10

                    ImageView {
                        id: invokedFavicon
                        objectName: "invokedFavicon"
                    	imageSource: item ? "file://" + item.favicon : "asset:///images/favicon.png"
                    	minHeight: 26
                    	minWidth: 26
                    	translationY: 5
                        visible: invokedURL.text.toString().length > 0
                    }
                    
                    Label {
                        id: invokedURL
                        objectName: "invokedURL"
                        text: domain(item.url)
//                        text: "www.bbornot2b.com"
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
            }
            
            Container {
                layout: DockLayout {}
                topPadding: 35
                bottomPadding: 35
                rightPadding: 10
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Bottom
                
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    
                    ImageView {
                        id: keepStar
                        imageSource: "asset:///images/keep.png"
                        scalingMethod: ScalingMethod.AspectFit
                        scaleX: 0.7
                        scaleY: 0.7
                    }
                    
                    Label {
                        text: "Favorite"
                        verticalAlignment: VerticalAlignment.Center
                        translationX: -10
                    }
                    
                    Label {
                        text: "(keep after read)"
                        visible: !app.getKeepAfterRead()
                        textStyle.color: Color.LightGray
                        verticalAlignment: VerticalAlignment.Center
                        translationX: -20
                        translationY: 2
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
                        app.keepBookmark(invokedURL.text, checked)
                        keepStar.imageSource = "asset:///images/menuicons/zip" + (checked ? "On" : "") + ".png"
                    }
                    onInvokeCheckedChanged: keepCheck.checked = invokeChecked
                }
            }
        }
    }	        	                   
}
