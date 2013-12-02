
import bb.cascades 1.0
  
Page {
    
    titleBar: TitleBar {
        title: "My Backpack"
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
                        title: ListItemData
                        property string dateTitle
                        
                        onTitleChanged: {
                            if (title != dateTitle) {
                                formatDate()
                            }
                        }
                        onDateTitleChanged: {
                            if (title != dateTitle) {
                                title = dateTitle
                            }
                        }
                        
                        function formatDate() {
                            if (title.substring(4,5) == "-") {
                                var today = new Date();
                                today = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 0, 0, 0, 0);
                                var header = new Date(title.substring(0,4), title.substring(5,7) - 1, title.substring(8,10), 0, 0, 0, 0);
                                var hours = (today.getTime() - header.getTime()) / 1000 / 60 / 60;
                                switch ((hours - hours % 24) / 24) {
                                    case 0:
                                        dateTitle = "Today";
                                        break;
                                    case 1:
                                        dateTitle = "Yesterday";
                                        break;
                                    default:
                                        dateTitle = header.toDateString();
                                        break;
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
                                    onTriggered: bookmark.ListItem.view.deleteBookmark(ListItemData.url)
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
                                    topPadding: 12
                                    rightPadding: 12
	                                ImageView {
	                                    id: iconImage
//	                                    imageSource: "asset:///images/favicon.png"
                                        imageSource: ListItemData.favicon
                                        minWidth: 48
                                        minHeight: 48
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
	                            }
                                Label {
                                    id: timeLabel
                                    text: ListItemData.size
                                    textStyle.color: Color.create("#07b1e6")
                                    horizontalAlignment: HorizontalAlignment.Right
                                    onCreationCompleted: formatTime()
                                    onTextChanged: formatTime()
                                    function formatTime() {
                                        if (text.indexOf("min") < 0) {
                                            var k10 = (text - text % 10000) / 10000
                                            switch (k10) {
                                                case 0:
                                                    text = "< 1 min"
                                                    break;
                                                default:
                                                    text = k10 + " min" 
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
            
            function deleteBookmark(url) {	                
                app.removeBookmark(url, true);
            }
            
            function toggleKeep(url, keep) {
                app.keepBookmark(url, keep)
            }     
        }
	}
}
