
import bb.cascades 1.3
import bb.system 1.0
import bb 1.3

Page {
    id: about
    objectName: "aboutSheet"
    
    signal close();
    
    property double taken: app.getOfflineDirSize()
    property double device: storageInfo.physicalCapacity()
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
            topPadding: ui.sdu(4)
            rightPadding: ui.sdu(4)
            bottomPadding: ui.sdu(4)
            leftPadding: ui.sdu(4)
            horizontalAlignment: HorizontalAlignment.Fill
            
            ImageView {
                imageSource: "asset:///images/icon.png"
                minWidth: 114
                scalingMethod: ScalingMethod.AspectFit
                horizontalAlignment: HorizontalAlignment.Right
            }
            
			Container {

                Label {
                text: "Backpack 3.0"
                textStyle.fontWeight: FontWeight.Bold
                    textStyle.fontSize: FontSize.Large
                }

                Container {
                    topMargin: ui.sdu(4)

                    Label {
                        text: "Storage"
                        textStyle.fontSize: FontSize.XSmall
                    }
                    
                    ProgressIndicator {
                        id: takenIndicator
                        value: taken / (space + taken)
                        topMargin: ui.sdu(2)
                        bottomMargin: ui.sdu(1)
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
                    + "\n- New design of user interface"
                    + "\n- New reading preamble screen"
                    + "\n- Support for 10.3 Passport, Classic and Leap"
                }
                
                Container {
                    topPadding: ui.sdu(2)
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    Button {
                        text: "Rate it!"
                        horizontalAlignment: HorizontalAlignment.Fill
                        attachedObjects: [
                            Invocation {
                                id: invoke
                                query: InvokeQuery {
                                    mimeType: "application/x-bb-appworld"
                                    uri: "appworld://content/20399673"
                                }
                            }
                        ]
                        onClicked: invoke.trigger("bb.action.OPEN")
                    }
                }
            }            
	    }
	}
}
