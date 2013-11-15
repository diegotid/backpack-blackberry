
import bb.cascades 1.0

Page {
    id: invokedForm
    objectName: "invokedForm"

    property variant item
    signal close();
    
    titleBar: TitleBar {
        title: "My Backpack"
        
        dismissAction: ActionItem {
            id: dismissButton
            objectName: "dismissButton"
            title: "Close"

            onTriggered: {
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
            background: background.imagePaint

            Container { // Status
			    topPadding: 25
			    leftPadding: 40
			    
			    Label {
			        id: status
			        objectName: "status"
                    textStyle.color: Color.create("#07b1e6")
                    textStyle.fontSize: FontSize.Large
//                    text: "Comment this for release"
                }			
			}
			
			Container { // Form
                layout: DockLayout {}
       
		        ImageView {
		            imageSource: "asset:///images/form_background.png"
		        }
        
				Container { // Activity & title
				    layout: StackLayout {
	                    orientation: LayoutOrientation.LeftToRight
	                }
                    topPadding: 80
                    rightPadding: 90
                    leftPadding: 100
                    maxHeight: 225
	                
	                Container {
	                    objectName: "activity"
	                    topPadding: 25
	                    bottomPadding: 20
	                    leftPadding: 20
	                    rightPadding: 40
	                    
	                    ActivityIndicator {
	                        running: true
	                        scaleX: 2
	                        scaleY: 2
	                    }             
	                    visible: !item
//	                    visible: false // Comment this for release  
	                }
	                
	                Container {
                        layout: DockLayout {}
	                    
		                Label {
		                    id: title
		                    objectName: "title"
		                    textStyle.fontSize: FontSize.XLarge
		                    textStyle.color: Color.White
		                    verticalAlignment: VerticalAlignment.Center
		                    multiline: true
                            text: item.title
//		                    text: "Comment this for release but must be at least three lines adslfkjasñdlkfjasñld  fñlaskdjf a"
		                }                                           
                    }
				}
				
				Container { // Memo & keep switch
                    verticalAlignment: VerticalAlignment.Bottom
                    rightPadding: 90
                    bottomPadding: 80
                    leftPadding: 90
				
					Container { // Memo
		                layout: DockLayout {}
	                    
                        ImageView {
                            imageSource: "asset:///images/memo_background.png"
			            }
		                
		                TextArea {
			                id: memo
			                objectName: "memo"
                            backgroundVisible: false
                            verticalAlignment: VerticalAlignment.Fill

                            text: item.memo
			                hintText: "Type your memo here (optional)"
//                          hintText: "Comment this for release but must be at least three lines Comment this for release but must be at least three lines"
//							hintText: "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum"

			                onTextChanging: {
                                if (focused) { // Don't do this when opened from the edit action
				                    acceptButton.enabled = (memo.text != "");
				                    dismissButton.title = (memo.text != "") ? "Cancel" : "Close";
				                }
	                            memo.textStyle.color = (memo.text != "") ? Color.create("#009fd3") : Color.White;
			                }
                        }
		            }
		                
		            Container {
		                layout: DockLayout {}
		                topPadding: 20
		                rightPadding: 5
		                bottomPadding: 15
		                leftPadding: 60
		                horizontalAlignment: HorizontalAlignment.Fill
		                
                        ImageView {
                            imageSource: "asset:///images/menuicons/zip.png"
                            translationX: -80
                            translationY: 10
                            scaleX: 0.9
                            scaleY: 0.9
                        }
                        
                        Label {
                            id: keepLabel
                            objectName: "keepLabel"
                            text: "Keep after read"
                            textStyle.color: Color.LightGray
                            verticalAlignment: VerticalAlignment.Center
                        }
		                
		                ToggleButton {
		                    id: keepCheck
		                    objectName: "keepCheck"
		                    horizontalAlignment: HorizontalAlignment.Right                  
		                    verticalAlignment: VerticalAlignment.Center
		                    checked: item && item.keep == "true"
		                    property bool invokeChecked: false
		                    
		                    onCheckedChanged: item ? app.keepBookmark(item.url, checked) : app.keepBookmark(checked)
                            onInvokeCheckedChanged: keepCheck.checked = invokeChecked
		                }
		            }
                }
            }	        	                   
	    }
    }
}    
