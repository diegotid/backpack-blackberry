
import bb.cascades 1.2
import bb.system 1.0
import bb 1.0

Page {
    id: about
    objectName: "aboutSheet"
    
    signal close();
    
    property double taken: app.getOfflineDirSize()
    property double device: storageInfo.fileSystemCapacity(app.getOfflineDir())
    property double space: storageInfo.availableFileSystemSpace(app.getOfflineDir())
    
    onTakenChanged: updateFigures()
    
    function updateFigures() {
        about.space = storageInfo.availableFileSystemSpace(app.getOfflineDir())
        takenIndicator.value = about.taken / (about.space + about.taken)
        takenLabel.text = getUISpace(about.taken)
        totalLabel.text = "(Device " + getUISpace(about.device) + ", free " + getUISpace(about.space) + ")"
    }
    
    titleBar: TitleBar {
        title: "About"
        dismissAction: ActionItem {
            title: "Close"
            onTriggered: about.close();   
        }
    }
    
    attachedObjects: [    
        FileSystemInfo {
            id: storageInfo
        }
    ]
    
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

    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
        Container {
            layout: DockLayout {}
            topPadding: 40
            rightPadding: 40
            bottomPadding: 40
            leftPadding: 40
            horizontalAlignment: HorizontalAlignment.Fill
            
            ImageView {
                imageSource: "asset:///images/icon.png"
                minWidth: 114
                scalingMethod: ScalingMethod.AspectFit
                horizontalAlignment: HorizontalAlignment.Right
            }
            
			Container {

                Label {
                    text: "Backpack 3.1"
                    textStyle.fontSize: FontSize.Large
                }

                Container {
                    topMargin: 40

                    Label {
                        text: "Storage"
                        textStyle.fontSize: FontSize.XSmall
                    }
                    
                    ProgressIndicator {
                        id: takenIndicator
                        value: taken / (space + taken)
                        topMargin: 20
                        bottomMargin: 10
                    }
                    
                    Container {
                        layout: DockLayout {}
                        horizontalAlignment: HorizontalAlignment.Fill

                        Label {
                            id: takenLabel
                            text: getUISpace(taken)
                        }
                        Label {
                            id: totalLabel
                            text: "(Device " + getUISpace(device) + ", free " + getUISpace(space) + ")"
                            textStyle.fontSize: FontSize.XSmall
                            horizontalAlignment: HorizontalAlignment.Right
                            textStyle.color: Color.LightGray
                        }
                    }
                }                
                
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    topPadding: 30
                    
                    Button {
                        id: cleanButton
                        text: "Empty your Backpack"
                        enabled: !username && taken > 0
                        horizontalAlignment: HorizontalAlignment.Fill
                        attachedObjects: [
                            SystemDialog {
                                id: cleanDialog
                                title: "Empty your Backpack"
                                body: "This action will remove all contents from your Backpack. Are you sure to proceed? (This action can't be undone)"
                                onFinished: {
                                    if (result == SystemUiResult.ConfirmButtonSelection) {
                                        app.emptyBackapck()
                                        about.taken = app.getOfflineDirSize()
                                        cleanButton.enabled = false
                                    } else {
                                        cleanDialog.close()
                                    }
                                }
                            }
                        ]
                        onClicked: cleanDialog.show()
                    }
                    
                    Label {
                        text: "You need to disconnect from Pocket before you can empty your Backpack"
                        textStyle.fontSize: FontSize.XXSmall
                        topMargin: 0
                        textStyle.color: Color.LightGray
                        visible: username
                    }
                }
            
                Label {
                    multiline: true
                    text: "New in this version:"
                    + "\n- Offline mode"
                    + "\n- Lots of improvements"
                }
                
                Container {
                    topPadding: 20
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    Button {
                        text: "Rate it!"
                        horizontalAlignment: HorizontalAlignment.Fill
                        onClicked: app.launchRating()
                    }
                }
            }            
	    }
	}
}
