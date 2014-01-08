
import bb.cascades 1.0
import bb.system 1.0
  
Page {
    
    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            FreeTitleBar {
                id: freeTitleBar
            }
        }
        scrollBehavior: TitleBarScrollBehavior.Sticky // Comment for 10.0
    }
    
    function updateUsername(username) {
        freeTitleBar.username = username
    }
    
    onCreationCompleted: loadBackground()
        
    function loadBackground() {
        backgroundRed.opacity = app.getBackgroundColour("red");
        backgroundGreen.opacity = app.getBackgroundColour("green");
        backgroundBlue.opacity = app.getBackgroundColour("blue");
        backgroundBase.opacity = app.getBackgroundColour("base");
        invokedForm.loadBackground()
    }
    
    attachedObjects: [
        Sheet {
            id: bookmarkSheet
            objectName: "bookmarkSheet"
            InvokedForm {
                id: invokedForm
                objectName: "invokedForm"
                onClose: bookmarkSheet.close();
            }
        }
    ]
    
    Container {
        layout: DockLayout {}
        
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
               
        ListView {
            id: bookmarks
            objectName: "bookmarks"

//            dataModel: XmlDataModel {
//                source: "debug.xml"
//            }
            
            listItemComponents: [
                
                ListItemComponent {
                    type: "header"
                    
                    Header {
                        title: formatDate(ListItemData)
                        function formatDate(dbDate) {
                            var strDate = dbDate.toString();
                            if (strDate.substring(4,5) == "-") {
                                var today = new Date();
                                today = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 0, 0, 0, 0);
                                var header = new Date(strDate.substring(0,4), strDate.substring(5,7) - 1, strDate.substring(8,10), 0, 0, 0, 0);
                                var hours = (today.getTime() - header.getTime()) / 1000 / 60 / 60;
                                switch ((hours - hours % 24) / 24) {
                                    case 0: return "Today";
                                    case 1: return "Yesterday";
                                    default: return header.toDateString();
                                }
                            }
                        }
                    }
                },
                
                ListItemComponent {
                    type: "item"
                    
                    Container {
                        id: bookmark
                        layout: DockLayout {}
                        background: background.imagePaint
                        
                        attachedObjects: [
                            ImagePaintDefinition {
                                id: background
                                imageSource: "asset:///images/row_shadow.png"
                                repeatPattern: RepeatPattern.X
                            }
                        ]
                        
                        contextActions: [
                            ActionSet {
                                ActionItem {
                                    title: "Edit"
                                    imageSource: "asset:///images/menuicons/ic_edit.png"
                                    onTriggered: bookmark.ListItem.view.openEditSheet(ListItemData)
                                }
                                ActionItem {
                                    title: "Toggle keep after read"
                                    imageSource: "asset:///images/menuicons/zip.png"
                                    onTriggered: bookmark.ListItem.view.toggleKeep(ListItemData.url, ListItemData.keep == "false")   
                                }
                                DeleteActionItem {
                                    onTriggered: {
                                        bookmark.ListItem.view.dataModel.remove(ListItemData)
                                        deletedBookmark.deletedItem = ListItemData
                                        deletedBookmark.show()
                                    }                                   
                                    attachedObjects: [
                                        SystemToast {
                                            id: deletedBookmark
                                            body: "Bookmark deleted: " + deletedItem.title
                                            button.label: "Undo"
                                            button.enabled: true 
                                            property variant deletedItem
                                            onFinished: {
                                                if (result == SystemUiResult.ButtonSelection) {
                                                    bookmark.ListItem.view.dataModel.insert(deletedItem)
                                                } else {
                                                    bookmark.ListItem.view.deleteBookmark(deletedItem)
                                                }
                                            }
                                        }
                                    ]
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
                            topPadding: 8
                            bottomPadding: 12
                            rightPadding: 8
                            leftPadding: 18
                            
                            Container {
                                layout: StackLayout {
                                    orientation: LayoutOrientation.LeftToRight
                                }
                                leftPadding: 6
                                
                                Container {
                                    id: iconContainer
                                    visible: false
                                    topPadding: 15
                                    rightPadding: 10
	                                ImageView {
	                                    id: iconImage
                                        imageSource: ListItemData.favicon
                                        minWidth: 42
                                        minHeight: 42
                                        onImageSourceChanged: {
                                            activity.visible = ListItemData.title ? false : true
                                        }
	                                }                            
                                }
                                
                                Container {
                                    id: activity
                                    visible: (ListItemData.title && ListItemData.favicon) ? false : true
                                    topPadding: 15
                                    leftPadding: 4
                                    rightPadding: 12
                                    scaleX: 1.3
                                    scaleY: 1.3
                                    onVisibleChanged: {
                                        iconContainer.visible = !activity.visible
                                    }
                                    ActivityIndicator {
                                        running: true
                                    }
                                }

                                Label {
                                    id: bookmarkTitle
                                    text: ListItemData.title
                                    textStyle.fontSize: FontSize.XLarge
                                    preferredWidth: 650
                                    translationX: -7
                                    onTextChanged: {
                                        activity.visible = ListItemData.favicon ? false : true
                                    }
                                }
                            }
                            
                            Container {
                                layout: DockLayout {}
                                horizontalAlignment: HorizontalAlignment.Fill
                                translationY: -3
                                Label {
                                    text: ListItemData.memo ? ListItemData.memo : ListItemData.url
	                                textStyle.color: ListItemData.memo ? Color.create("#07b1e6") : Color.Gray
	                                textStyle.fontSize: FontSize.Medium
	                                multiline: ListItemData.memo
	                                preferredWidth: 560
	                            }                            
                                Label {
                                    id: timeLabel
                                    textStyle.color: Color.create("#07b1e6")
                                    textStyle.fontSize: FontSize.Medium
                                    horizontalAlignment: HorizontalAlignment.Right
                                    text: formatTime(ListItemData.size)
                                    function formatTime(size) {
                                        if (size.toString().indexOf("min") < 0) {
                                            if (size == 0)
                                            	return "";
                                            var k10 = (size - size % 10000) / 10000
                                            switch (k10) {
                                                case 0: return "< 1 min"
                                                default: return k10 + " min" 
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        ImageView {
                            imageSource: "asset:///images/keep.png"
                            visible: ListItemData.keep
                            horizontalAlignment: HorizontalAlignment.Right
                            translationX: -10
                            translationY: 24
                            onCreationCompleted: timeLabel.visible = !visible
                            onVisibleChanged: timeLabel.visible = !visible
                        }
                        
                        Divider {
                            topMargin: 0
                            bottomMargin: 0
                        }
                        
                        onTouch: {
                            if (event.touchType == TouchType.Down)
                                highlight.visible = true
                            else if (event.touchType == TouchType.Cancel || event.touchType == TouchType.Up)
                                highlight.visible = false
                        }
                    }                        
                }
            ]
                       
            onTriggered: {
                var selectedItem = dataModel.data(indexPath)
                app.browseBookmark(selectedItem.url)
                app.removeBookmark(selectedItem.url)
            }

            function openEditSheet(row) {
                invokedForm.item = row
                bookmarkSheet.open()
            }
            
            function deleteBookmark(ListItemData) {
                app.removeBookmark(ListItemData.url, true);
            }
            
            function toggleKeep(url, keep) {
                app.keepBookmark(url, keep)
            }     
        }
	}
}
