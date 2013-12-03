
import bb.cascades 1.0
import bb.cascades.pickers 1.0

Page {
    id: backup
    
    signal close();
    
    titleBar: TitleBar {
        title: "Backup/Restore"
        dismissAction: ActionItem {
            title: "Close"
            onTriggered: backup.close();   
        }
    }
    
    attachedObjects: [
        FilePicker {
            id: importFilePicker
            type: FileType.Other
            title: "Select import file"
            directories: ["/accounts/1000/shared"]
            onFileSelected: app.importBackupFile(selectedFiles[0])
        }
    ]
    
    Container {
        
        Header {
            title: "Backup"
            subtitle: "Save Backpack content for later use on this or other device"
        }
        
        Container {
            topPadding: 20.0
            leftPadding: 20.0
            rightPadding: 20.0
            bottomPadding: 20.0
            layout: DockLayout {}
            horizontalAlignment: HorizontalAlignment.Fill

            Button {
                text: "Import from file"
                horizontalAlignment: HorizontalAlignment.Left
                minWidth: 300
                onClicked: importFilePicker.open()
            }

            Button {
                text: "New backup"
                horizontalAlignment: HorizontalAlignment.Right
                minWidth: 300
                onClicked: app.saveBackup();
            }
        }
        
        Header {
            title: "Restore"
            subtitle: "Restore previous backups, or exchange backup files"
        }
        
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            topPadding: 10

			Container {
                objectName: "lastBackupStatus"
                topPadding: 10
                leftPadding: 20
                rightPadding: 20
                bottomPadding: 10

	            Label {
	                text: "No previous backups"
	            }        
            }
            
            ListView {
                id: backupsList
                objectName: "backupsList"

//                dataModel: XmlDataModel {
//                    source: "debug.xml"
//                }

                listItemComponents: [
                                           
                    ListItemComponent {
                        type: "item"
                            
		                Container {
                            id: backupItem
                            layout: DockLayout {}

                            contextActions: [
                                ActionSet {
                                    DeleteActionItem {
                                        onTriggered: backupItem.ListItem.view.deleteBackup(ListItemData.file)
                                    }
                                }
                            ]
                            
                            Container {
                                id: highlight
                                verticalAlignment: VerticalAlignment.Fill
                                horizontalAlignment: HorizontalAlignment.Fill
                                background: highlightBorder.imagePaint
                                visible: false
                                
                                attachedObjects: [
                                    ImagePaintDefinition {
                                        id: highlightBorder
                                        imageSource: "asset:///images/highlight_listitem.amd"
                                        repeatPattern: RepeatPattern.Fill
                                    }
                                ]
                            }
                            
                            Container {
                                topPadding: 10
                                leftPadding: 20
                                rightPadding: 20
                                bottomPadding: 10
                                layout: DockLayout {}
                                preferredWidth: 768

								Container {
                                    verticalAlignment: VerticalAlignment.Center
                                    preferredWidth: 355
                                    preferredHeight: 90
                                    layout: DockLayout {}

                                    Label {
                                        verticalAlignment: VerticalAlignment.Center
                                        text: ListItemData.date
                                    }
                                    onTouch: {
                                        if (event.touchType == TouchType.Down)
                                            highlight.visible = true
                                        else if (event.touchType == TouchType.Cancel || event.touchType == TouchType.Up)
                                            highlight.visible = false
                                    }
                                }
                                
                                Container {
                                    maxWidth: 355
                                    horizontalAlignment: HorizontalAlignment.Right
                                    layout: StackLayout {
                                        orientation: LayoutOrientation.LeftToRight
                                    }
                                    Button {
                                        text: "Restore"
                                        onClicked: backupItem.ListItem.view.restoreBackup(ListItemData.file)
                                    }
                                    Button {
                                        imageSource: "asset:///images/menuicons/ic_share.png"
                                        maxWidth: 120
                                        onClicked: backupItem.ListItem.view.shareBackup(ListItemData.file)
                                    }
                                }
                            }
                        }
                    }
                ]
                
                function restoreBackup(file) {
                    app.restoreBackup(file)
                }
                
                function shareBackup(file) {
                    app.shareBackup(file)
                }
                
                function deleteBackup(file) {
                    app.deleteBackup(file)
                }
            }            
        }
    }
}