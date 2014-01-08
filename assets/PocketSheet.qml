
import bb.cascades 1.0
import bb.system 1.0

Page {
    id: pocket
    objectName: "pocketPage"
    
    signal close();
    
    property string state    
    onStateChanged: {
        if (state == "sync") {
            app.pocketConnect()
        }
        closeButton.title = (state == "on" || state == "off") ? "Close" : "Cancel"
    }

    property string username
    onUsernameChanged: {
        state = "on"
    }
    
    titleBar: TitleBar {
        title: "Pocket sync"
        dismissAction: ActionItem {
            id: closeButton
            title: "Cancel"
            onTriggered: {
                state = ""
                pocket.close()
            }   
        }
    }
    
    property string error
    onErrorChanged: {
        state = ""
        pocketError.errorMessage = error
        pocketError.show()
    }

    attachedObjects: [
        Invocation {
            id: pocketLink
            query {
                mimeType: "text/html"
                uri: "http://getpocket.com"
            }
        },
        SystemToast {
            id: pocketError
            property string errorMessage
            body: "Pocket server said: " + errorMessage
            button.label: "Close"
            button.enabled: true 
            onFinished: pocket.close()
        }
    ]
    
    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
		Container {
		    bottomPadding: 30
	
	        ImageView {
	            imageSource: "asset:///images/pocket.png"
	            scalingMethod: ScalingMethod.AspectFit
	            bottomMargin: 0
	        }
	            
	        Container {
	            topPadding: 0
	            rightPadding: 30
	            bottomPadding: 0
	            leftPadding: 30
	            horizontalAlignment: HorizontalAlignment.Fill
	
	            Container {
	                visible: state == "why"
	                horizontalAlignment: HorizontalAlignment.Fill
	                bottomMargin: 30
	                
	                Container {
	                    leftPadding: 10
	                    rightPadding: 10
	                    
	                    Label {
	                        multiline: true
	                        text: "Pocket (getpocket.com) is the application, available as web browser extensions and on other mobile platforms, that lets you save content to read it later on any supported device."
	                    }
	                    
	                    Label {
	                        multiline: true
	                        text: "By syncing Backpack with Pocket you will have access from your handheld to the content you saved on your computer, or will be able to read on your laptop what you've shared with Backpack on your BlackBerry."                     
	                    }
	                }            
	                
	                Container {
	                    topPadding: 60
	                    horizontalAlignment: HorizontalAlignment.Fill
	                    
	                    Button {
	                        text: "Sounds great! Sync with Pocket"
	                        horizontalAlignment: HorizontalAlignment.Fill
	                        onClicked: {
	                            state = "sync"
	                        }
	                    }
	                    
	                    Label {
                            text: "<span style='text-decoration:underline'>More about Pocket</span>"
	                        textFormat: TextFormat.Html
	                        horizontalAlignment: HorizontalAlignment.Right
	                        textStyle.fontSize: FontSize.Small
	                        onTouch: pocketLink.trigger("bb.action.OPEN")
	                    }            
	                }
	            }
                
                Container {
                    visible: state == "sync"
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomMargin: 30
                    
                    Container {
                        leftPadding: 10
                        rightPadding: 10
                        
                        Label {
                            multiline: true
                            text: "You are being redirected to Pocket on your browser to log in with your account and authorize Backpack to sync"
                        }
                    }            
                    
                    Container {
                        topPadding: 60
                        horizontalAlignment: HorizontalAlignment.Fill
                        
                        Button {
                            text: "I'm done! Complete and sync"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                app.pocketCompleteAuth()
                                state = "on"
                                syncingIndicator.visible = true
                            }
                        }
                    }            
                }
                
                Container {
                    visible: state == "on"
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomMargin: 30
                    
                    Container {
                        leftPadding: 10
                        rightPadding: 10
                        
                        horizontalAlignment: HorizontalAlignment.Fill
                        Label {
                            text: "You are connected to Pocket as:"
                        }
                        
                        Container {
                            bottomPadding: 50
                            horizontalAlignment: HorizontalAlignment.Fill
                            
                            Label {
                                text: username
                                textStyle.fontSize: FontSize.Large
                                textStyle.fontWeight: FontWeight.Bold
                                horizontalAlignment: HorizontalAlignment.Center
                            }
                        }
                        
                        Container {
                            id: syncingIndicator
                            objectName: "syncingIndicator"
                            layout: DockLayout {}
                            visible: false
                            onVisibleChanged: disconnectButton.enabled = !syncingIndicator.visible

                            horizontalAlignment: HorizontalAlignment.Fill
                            Label {
                                text: "Syncing content..."
                            }
                            
                            ActivityIndicator {
                                running: true
                                horizontalAlignment: HorizontalAlignment.Right
                                verticalAlignment: VerticalAlignment.Bottom
                            }
                        }
                    }            
                    
                    Container {
                        topPadding: 30
                        horizontalAlignment: HorizontalAlignment.Fill
                        
                        Button {
                            id: disconnectButton
                            text: "Disconnect or switch account"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                state = "off"
                                app.pocketDisconnect()
                            }
                        }
                    }            
                }
                
                Container {
                    visible: state == "off"
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomMargin: 30
                    
                    Container {
                        leftPadding: 10
                        rightPadding: 10
                        
                        horizontalAlignment: HorizontalAlignment.Fill
                        Label {
                            text: "You've just been disconnected as <b>" + username + "</b>"
                            textFormat: TextFormat.Html
                            multiline: true
                        }
                        
                        Label {
                            multiline: true
                            text: "Previously synced content will stay on your device unless you clean it now."                     
                        }
                        
                        Label {
                            multiline: true
                            text: "In order to connect to a different Pocket account, you'll previously need to visit getpocket.com and perform a logout from your browser."
                        }
                    }            
                    
                    Container {
                        topPadding: 30
                        horizontalAlignment: HorizontalAlignment.Fill
                        
                        Button {
                            text: "Clean " + username + "'s content"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                app.pocketCleanContent()
                            }
                        }
                        
                        Button {
                            text: "Logout on getpocket.com"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: pocketLink.trigger("bb.action.OPEN")
                        }
                        
                        Button {
                            text: "Sync again"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                state = "sync"
                            }
                        }
                    }            
                }
	        }
		}
	}
}