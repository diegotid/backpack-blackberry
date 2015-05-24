
import bb.cascades 1.4
import bb.system 1.0

Page {
    id: pocket
    objectName: "pocketPage"
    
    signal close();
    
    property string state
//    property string state: "on"
//    property string state: "off"
//    property string state: "why"
//    property string state: "sync"

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
            body: "Pocket: " + errorMessage
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
	            topPadding: ui.du(2)
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
	                        onTouch: {
                                if (event.touchType == TouchType.Down) {
                                    pocketLink.trigger("bb.action.OPEN")
                                }
                            }
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
                                cleanButton.enabled = true
                                logoutButton.enabled = true
                            }
                        }
                    }            
                }
                
                Container {
                    visible: state == "on"
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomMargin: 30
                    
                    Container {
                        leftPadding: 5
                        rightPadding: 10
                        horizontalAlignment: HorizontalAlignment.Fill
                        
                        Container {
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                            bottomPadding: 10
                            visible: username.length > 0

                            Label {
                                text: "Connected as:"
                                verticalAlignment: VerticalAlignment.Bottom
                                translationY: -3
                            }
                            
                            Label {
                                text: username
                                textStyle.fontSize: FontSize.Large
                                textStyle.fontWeight: FontWeight.Bold
                                verticalAlignment: VerticalAlignment.Bottom
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
                                text: username.length > 0 ? "Syncing content..." : "Connecting to Pocket..."
                            }
                            
                            ActivityIndicator {
                                running: true
                                horizontalAlignment: HorizontalAlignment.Right
                                verticalAlignment: VerticalAlignment.Bottom
                            }
                        }
                    }
                    
                    Container {
                        visible: username.length > 0 && syncingIndicator.visible == false
                        horizontalAlignment: HorizontalAlignment.Fill

                        Container {
                            layout: DockLayout {}
                            leftPadding: 5
                            bottomPadding: 10
                            horizontalAlignment: HorizontalAlignment.Fill

                            Label {
                                text: "Sync on app startup"
                                verticalAlignment: VerticalAlignment.Center
                            }
                            
                            ToggleButton {
                                horizontalAlignment: HorizontalAlignment.Right
                                checked: app.pocketGetSynconstartup()
                                onCheckedChanged: app.pocketSetSynconstartup(checked)
                            }
                        }
                        
                        DropDown {
                            title: "Automatically sync"
                            options: [
                                Option {
                                    text: "Never"
                                    value: 0
                                    selected: app.pocketInterval() == value
                                },
                                Option {
                                    text: "Every 1h"
                                    value: 1
                                    selected: app.pocketInterval() == value
                                },
                                Option {
                                    text: "Every 6h"
                                    value: 6
                                    selected: app.pocketInterval() == value
                                },
                                Option {
                                    text: "Every 12h"
                                    value: 12
                                    selected: app.pocketInterval() == value
                                }
                            ]
                            onSelectedOptionChanged: app.pocketSetInterval(selectedOption.value)
                        }
                        
                        Button {
                            text: "Sync now"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                syncingIndicator.visible = true
                                app.pocketRetrieve()
                            }
                        }
                        
                        Button {
                            id: disconnectButton
                            text: "Disconnect or switch account"
                            horizontalAlignment: HorizontalAlignment.Fill
                            attachedObjects: [
                                SystemDialog {
                                    id: disconnectDialog
                                    title: "Disconnect from Pocket"
                                    body: "Are you sure you want to disconnect?"
                                    onFinished: {
                                        if (result == SystemUiResult.ConfirmButtonSelection) {
                                            state = "off"
                                            app.pocketDisconnect()
                                        } else {
                                            disconnectDialog.close()
                                        }
                                    }
                                }
                            ]
                            onClicked: disconnectDialog.show()
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
                            id: cleanButton
                            text: "Clean " + username + "'s content"
                            horizontalAlignment: HorizontalAlignment.Fill
                            attachedObjects: [
                                SystemDialog {
                                    id: cleanDialog
                                    title: "Clean Pocket content"
                                    body: "Are you sure you want to delete " + username + "'s content? (Pocket will still keep it)"
                                    onFinished: {
                                        if (result == SystemUiResult.ConfirmButtonSelection) {
                                            app.pocketCleanContent()
                                            cleanButton.enabled = false
                                        } else {
                                            cleanDialog.close()
                                        }
                                    }
                                }
                            ]
                            onClicked: cleanDialog.show()
                        }
                        
                        Button {
                            id: logoutButton
                            text: "Logout on getpocket.com"
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                pocketLink.trigger("bb.action.OPEN")
                                logoutButton.enabled = false
                            }
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