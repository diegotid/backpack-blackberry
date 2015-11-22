
import bb.cascades 1.3
import bb.system 1.0
import bb 1.3

Page {
    id: pocket
    objectName: "pocketPage"
    
    signal close();
    
    property string username
    
    property string state
//    property string state: "on"
//    property string state: "off"
//    property string state: "why"
//    property string state: "sync"   
 
    onStateChanged: {
        offline = app.getOfflineMode()
        offimages = app.getOfflineImages()
        offwifi = app.getOfflineWiFi()
        onwifi = app.onWiFi()
        if (state != "off" && username.length > 0) {
            state = "on"
        } else if (state == "sync") {
            app.pocketConnect()
        }
        closeButton.title = (state == "on" || state == "off") ? "Close" : "Cancel"
        completeButton.enabled = true
        updateCompleteButton()
    }
    
    property double space: storageInfo.availableFileSystemSpace(app.getOfflineDir())

    property bool offline
    onOfflineChanged: updateCompleteButton()
    
    property bool offimages
    onOffimagesChanged: updateFigures()
    
    property bool offwifi
    onOffwifiChanged: {
        updateCompleteButton()
        wifiState.text = "You are not connected to a Wi-Fi network. " + (offwifi ? "Offline content will be downloaded when a Wi-Fi connection is available" : "Please consider your mobile data plan before downloading")
    }
    
    function updateCompleteButton() {
        if (offline && !completeButton.finished) {
            completeButton.text = "Complete and check download size"
        } else if (completeButton.finished) {
            completeButton.text = (offwifi && !onwifi) ? "Sync and download later" : "Sync and start download"
        } else {
            completeButton.text = "I'm done! Complete and sync"
        }
    }

    property bool onwifi
    property int images
    property int download
    property int fulldownload
    onFulldownloadChanged: updateFigures()
    
    function updateFigures() {
        download = fulldownload - (offimages ? 0 : images)
        if (download > 0) {
            downloadEstimated.text = "Estimated download size is " + getUISpace(download)
            if (download / space < 0.85) {
                completeButton.enabled = true
            }
            downloadCalculating.running = false
        }
        storageState.visible = download > 0
    }
    
    function getUISpace(given) {
        if (given < 1024 * 1024 / 10) {
            return (given / 1024).toFixed(1) + "KB"
        }
        given = given / 1024 / 1024
        if (given > 1024) {
            return (given / 1024).toFixed(1) + "GB"
        } else {
            return given.toFixed(1) + "MB"
        }
    }
    
    function cancelSync(title) {        
        if (title == "Cancel") {
            app.pocketDisconnect()
            images = 0
            fulldownload = 0
            username = ""
            downloadEstimated.text = ""
            completeButton.finished = false
            completeButton.text = offline ? "Complete and check download size" : "I'm done! Complete and sync"
        } else {
            mainPage.activeTab = exploreTab
            if (state == "off") {
                username = ""
            }
        }
        state = ""
        pocket.close()
    }

    titleBar: TitleBar {
        title: "Pocket sync"
        dismissAction: ActionItem {
            id: closeButton
            title: "Cancel"
            onTriggered: cancelSync(title)   
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
        },
        FileSystemInfo {
            id: storageInfo
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
	            bottomPadding: 0
                rightPadding: ui.sdu(4)
                leftPadding: ui.sdu(4)
	            horizontalAlignment: HorizontalAlignment.Fill
                
                Container {
	                visible: state == "why"
	                horizontalAlignment: HorizontalAlignment.Fill
	                bottomMargin: 30
	                
	                Container {
	                    
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
                    id: syncState
                    visible: state == "sync"
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomMargin: 30
                    
                    Container {
                        bottomPadding: 30
                        
                        Label {
                            multiline: true
                            visible: !offline || !completeButton.finished
                            text: username.length > 0 ? "Logged in as " + username + " to sync Pocket with offline mode off" : "You are being redirected to Pocket on your browser to log in with your account and authorize Backpack to sync"
                        }
                        
                        Label {
                            multiline: true
                            visible: offline
                            text: "Your offline mode is on" + (username.length > 0 ? (" as " + username) : (". Once logged in " + (onwifi || !offwifi ? "you will be ready for downloading your Pocket articles" : "your Pocket articles will download when connected to a Wi-Fi network")))
                        }
                    }            
                    
                    Container {
                        visible: offline && downloadEstimated.text.length > 0
                        horizontalAlignment: HorizontalAlignment.Fill
                        topPadding: 10
                        
                        Container {
                            layout: DockLayout {}
                            horizontalAlignment: HorizontalAlignment.Fill
                            
                            Label {
                                id: downloadEstimated
                            }
                            
                            ActivityIndicator {
                                id: downloadCalculating
                                horizontalAlignment: HorizontalAlignment.Right
                            }
                        }
                        
                        Container {
                            visible: download > 0
                            topPadding: ui.sdu(3)
                            bottomPadding: 0
                            horizontalAlignment: HorizontalAlignment.Fill
                            
                            ProgressIndicator {
                                id: spaceOccupied
                                value: download / space
                                visible: value < 0.85
                            }
                            
                            Container {
                                visible: download / space >= 0.85
                                background: Color.DarkRed
                                bottomPadding: ui.sdu(2)
                                horizontalAlignment: HorizontalAlignment.Fill
                                maxHeight: 6
                                Label {
                                    text: "Simulating a red progress indicator bar"
                                }
                            }
                        }
                        
                        Label {
                            id: storageState
                            multiline: true
                            visible: false
                            textStyle.fontSize: FontSize.XSmall
                            text: "Available space in your device is " + getUISpace(space)
                                + (onwifi ? " and you're connected to a Wi-Fi network" : "")
                                + (offimages ? (images > 999999 ? ". With no offline images, download size would be " + getUISpace(fulldownload - images) : "") : ", so you could also download images taking up " + getUISpace(fulldownload))
                        }
                        
                        Label {
                            id: storageFull
                            multiline: true
                            visible: download / space >= 0.85
                            textStyle.fontSize: FontSize.XSmall
                            text: download >= space ? "You don't have enough space in your device for syncing Pocket on offline mode" : "Syncing Pocket on offline mode would take up nearly all your device's free space"
                        }
                        
                        Label {
                            id: wifiState
                            multiline: true
                            visible: !onwifi && !storageFull.visible
                            text: "You are not connected to a Wi-Fi network. " + (offwifi ? "Offline content will be downloaded when a Wi-Fi connection is available" : "Please consider your mobile data plan before downloading")
                            textStyle.fontSize: FontSize.XSmall
                        }
                        
                        Container {
                            topPadding: 30
                            visible: storageState.visible || wifiState.visible
                            horizontalAlignment: HorizontalAlignment.Fill

                            Button {
                                text: "Check settings"
                                horizontalAlignment: HorizontalAlignment.Fill
                                onClicked: settingsSheet.open()
                            }
                        }
                    }
                    
                    Container {
                        topPadding: 30
                        horizontalAlignment: HorizontalAlignment.Fill
                        
                        Button {
                            id: completeButton
                            text: "I'm done! Complete and sync"
                            property bool finished: false
                            horizontalAlignment: HorizontalAlignment.Fill
                            onClicked: {
                                if (finished) {
                                    app.pocketRetrieve()
                                } else {
                                    app.pocketCompleteAuth()
                                }
                                if (offline && !finished) {
                                    text = (offwifi && !onwifi) ? "Sync and download later" : "Sync and start download"
                                    downloadEstimated.text = "Checking download size..."
                                    downloadCalculating.running = true
                                    finished = true
                                    enabled = false
                                } else {
                                    state = "on"
                                    images = 0
                                    fulldownload = 0
                                    finished = false
                                    cleanButton.enabled = true
                                    logoutButton.enabled = true                                    
                                    syncingIndicator.visible = true
                                    text = offline ? "Complete and check download size" : "I'm done! Complete and sync"
                                    downloadCalculating.running = false
                                    downloadEstimated.text = ""
                                }
                            }
                        }
                    }            
                }
                
                Container {
                    id: onState
                    visible: state == "on" && !syncState.visible
                    horizontalAlignment: HorizontalAlignment.Fill
                    bottomMargin: 30
                    
                    Container {
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
                                            app.emptyBackapck()
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
                                if (state == "off") {
                                    username = ""
                                }
                               state = "sync"
                            }
                        }
                    }            
                }
	        }
		}
	}
}