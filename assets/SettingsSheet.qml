
import bb.cascades 1.2
import bb.system 1.0

Page {
    id: settings
    objectName: "settingsSheet"
    
    signal close();

    property bool prevKeepAfterRead: app.getKeepAfterRead()
    property bool prevPocketDelete: app.getPocketDeleteMode()
    property bool prevOfflineMode: app.getOfflineMode()    
    property bool prevOfflineImages: app.getOfflineImages()
    property bool prevOfflineWiFi: app.getOfflineWiFi()
    property bool prevKeptShuffle: app.getIgnoreKeptShuffle()
    property bool prevKeptQuickest: app.getIgnoreKeptQuickest()
    property bool prevKeptLounge: app.getIgnoreKeptLounge()
    
    property int backpackCount
    onBackpackCountChanged: {
        offlineConfirm.body = "By activating your offline mode Backpack will download now your " + backpackCount + " articles using your data plan\n\nYou can also set download scheme to 'Only on Wi-Fi'"
    }

    titleBar: TitleBar {
        title: "Settings"
        dismissAction: ActionItem {
            title: "Cancel"
            onTriggered: {
                offlineMode.checked = prevOfflineMode
                offlineImages.setSelectedOption(offlineImages.at(prevOfflineImages))
                offlineWiFi.setSelectedOption(offlineWiFi.at(prevOfflineWiFi))
                keepAfterReadMode.setSelectedOption(keepAfterReadMode.at(prevKeepAfterRead))
                removeOnPocketMode.setSelectedOption(removeOnPocketMode.at(prevPocketDelete))
                ignoreKeptShuffle.checked = prevKeptShuffle
                ignoreKeptQuickest.checked = prevKeptQuickest
                ignoreKeptLounge.checked = prevKeptLounge
                settings.close()
            }
        }
        acceptAction: ActionItem {
            title: "Confirm"
            onTriggered: {
                app.setOfflineMode(offlineMode.checked)
                app.setOfflineImages(offlineImages.selectedValue)
                app.setOfflineWiFi(offlineWiFi.selectedValue)
                app.setKeepAfterRead(keepAfterReadMode.selectedValue)
                app.setPocketDeleteMode(removeOnPocketMode.selectedValue)
                app.setIgnoreKeptShuffle(ignoreKeptShuffle.checked)
                app.setIgnoreKeptQuickest(ignoreKeptQuickest.checked)
                app.setIgnoreKeptLounge(ignoreKeptLounge.checked)
                prevOfflineMode = offlineMode.checked
                prevOfflineImages = offlineImages.selectedValue
                prevOfflineWiFi = offlineWiFi.selectedValue
                prevKeepAfterRead = keepAfterReadMode.selectedValue
                prevPocketDelete = removeOnPocketMode.selectedValue
                prevKeptShuffle = ignoreKeptShuffle.checked
                prevKeptQuickest = ignoreKeptQuickest.checked
                prevKeptLounge = ignoreKeptLounge.checked
                settings.close()
            }
        }
    }
    
    attachedObjects: [
        SystemDialog {
            id: offlineConfirm
            title: "Offline mode"
            confirmButton.label: "Understood"
            onFinished: {
                if (result == SystemUiResult.CancelButtonSelection) {
                    offlineMode.checked = false
                    offlineWiFi.enabled = true
                }
            }
        }
    ]
    
    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
        Container {
            
            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 20
                leftPadding: 20
                rightPadding: 20
                bottomPadding: 20
                
                Container {                    
                    Label {
                        text: "Offline mode"
                        verticalAlignment: VerticalAlignment.Center
                        bottomMargin: 0
                    }
                    Label {
                        text: "(Takes up device's storage space)"
                        topMargin: 0
                        textStyle.fontSize: FontSize.XXSmall
                        textStyle.color: Color.LightGray
                    }
                }
                
                ToggleButton {
                    id: offlineMode
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getOfflineMode()
                    onCheckedChanged: {
                        if (checked && !prevOfflineMode
                                && !app.onWiFi()
                                && offlineWiFi.selectedValue == 0
                                && backpackCount > 0) {
                            offlineConfirm.show()
                        }
                    }
                }
            }
            
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 10
                
                DropDown {
                    id: offlineImages
                    title: "Offline content"
                    enabled: offlineMode.checked
                    options: [
                        Option {
                            text: "Only text"
                            value: 0
                            selected: !prevOfflineImages
                        },
                        Option {
                            text: "Text and images"
                            value: 1
                            selected: prevOfflineImages
                        }
                    ]
                }
                
                DropDown {
                    id: offlineWiFi
                    title: "Download scheme"
                    enabled: offlineMode.checked
                    options: [
                        Option {
                            text: "Always download"
                            value: 0
                            selected: !prevOfflineWiFi
                        },
                        Option {
                            text: "Only on Wi-Fi"
                            value: 1
                            selected: prevOfflineWiFi
                        }
                    ]
                }
            }
            
            Header {
                title: "Persistence options"
            }
            
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                leftPadding: 10
                rightPadding: 10
                topPadding: 20
                bottomPadding: 10
                
	            DropDown {
	                id: keepAfterReadMode
	                title: "Keep in Backpack after read"
	                options: [
                        Option {
                            text: "Only favorites"
                            value: 0
                            selected: !prevKeepAfterRead
                        },
                        Option {
                            text: "Everything"
                            value: 1
                            selected: prevKeepAfterRead
                        }
	                ]
	            }
	            
	            Label {
                    text: "Pocket action on read: Archive non-favorites"
                    visible: username && keepAfterReadMode.selectedOption.value < 1
                    textStyle.fontSize: FontSize.XSmall
                    topMargin: 0
                    translationY: -3
                    translationX: 20
                    textStyle.color: Color.LightGray
                }

                DropDown {
                    id: removeOnPocketMode
                    title: "Pocket action on Backpack delete"
                    enabled: username
                    options: [
                        Option {
                            text: "Archive"
                            value: 0
                            selected: !prevPocketDelete
                        },
                        Option {
                            text: "Delete"
                            value: 1
                            selected: prevPocketDelete
                        }
                    ]
                }
            }

            Header {
                title: "Ignore favorite bookmarks on mode..."
                subtitle: "(if any non-favorite)"
            }

            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                leftPadding: 20.0
                rightPadding: 20.0

                Label {
                    text: "Shuffle"
                    verticalAlignment: VerticalAlignment.Center
                }

                ToggleButton {
                    id: ignoreKeptShuffle
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getIgnoreKeptShuffle()
                }
            }

            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                leftPadding: 20.0
                rightPadding: 20.0

                Label {
                    text: "Quickest"
                    verticalAlignment: VerticalAlignment.Center
                }

                ToggleButton {
                    id: ignoreKeptQuickest
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getIgnoreKeptQuickest()
                }
            }

            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                leftPadding: 20.0
                rightPadding: 20.0
                bottomMargin: 10.0
                
                Label {
                    text: "Lounge"
                    verticalAlignment: VerticalAlignment.Center
                }
                
                ToggleButton {
                    id: ignoreKeptLounge
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getIgnoreKeptLounge()
                }
            }
	    }
	}
}
