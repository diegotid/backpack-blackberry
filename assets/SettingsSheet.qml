
import bb.cascades 1.0

Page {
    id: settings
    
    signal close();
    
    property bool prevKeepAfterRead: app.getKeepAfterRead()
    property bool prevPocketDelete: app.getPocketDeleteMode()
    property bool prevKeptShuffle: app.getIgnoreKeptShuffle()
    property bool prevKeptQuickest: app.getIgnoreKeptQuickest()
    property bool prevKeptLounge: app.getIgnoreKeptLounge()

    titleBar: TitleBar {
        title: "Settings"
        dismissAction: ActionItem {
            title: "Cancel"
            onTriggered: {
                keepAfterReadMode.setSelectedOption(keepAfterReadMode.at(prevKeepAfterRead))
                removeOnPocketMode.setSelectedOption(removeOnPocketMode.at(prevPocketDelete))
                ignoreKeptShuffle.checked = prevKeptShuffle
                ignoreKeptQuickest.checked = prevKeptQuickest
                ignoreKeptLounge.checked = prevKeptLounge
                settings.close();   
            }
        }
        acceptAction: ActionItem {
            title: "Confirm"
            onTriggered: {
                app.setKeepAfterRead(keepAfterReadMode.selectedValue)
                app.setPocketDeleteMode(removeOnPocketMode.selectedValue)
                app.setIgnoreKeptShuffle(ignoreKeptShuffle.checked)
                app.setIgnoreKeptQuickest(ignoreKeptQuickest.checked)
                app.setIgnoreKeptLounge(ignoreKeptLounge.checked)
                prevKeepAfterRead = keepAfterReadMode.selectedValue
                prevPocketDelete = removeOnPocketMode.selectedValue
                prevKeptShuffle = ignoreKeptShuffle.checked
                prevKeptQuickest = ignoreKeptQuickest.checked
                prevKeptLounge = ignoreKeptLounge.checked
                settings.close()
            }
        }
    }
    
    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
        Container {
            
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                leftPadding: 10
                rightPadding: 10
                topPadding: 25
                bottomPadding: 30
                
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
                    textStyle.fontSize: FontSize.Small
                    translationY: -10
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
                layout: DockLayout {
                }
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
