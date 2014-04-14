
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
    }
    
    function updateUsername(username) {
        freeTitleBar.username = username
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
        
        attachedObjects: LayoutUpdateHandler {
            id: pageHandler
            onLayoutFrameChanged: Qt.pageHandler = pageHandler
        }

        Container {
            id: bookmarksList
            layout: DockLayout {}
            topPadding: searchFormHandler.layoutFrame.height + 20
            
            ListView {
                id: bookmarks
                objectName: "bookmarks"
                
                onDataModelChanged: {
                	emptyLabelContainer.visible = (dataModel.childCount(0) == 0)
                	var tale = query.text.toString().trim().length > 0 ? " found" : ""
                    switch (filterType.selectedOption.value) {
                        case 1:
                            emptyLabel.text = "No favorited items" + tale
                            break
                        case 2:
                            emptyLabel.text = "No articles" + tale
                            break
                        case 3:
                            emptyLabel.text = "No videos" + tale
                            break
                        case 4:
                            emptyLabel.text = "No images" + tale
                            break
                        default:
                            emptyLabel.text = "No items" + tale
                    }
                }
                
//                dataModel: XmlDataModel {
//                    source: "debug.xml"
//                }
                
                listItemComponents: [
                    
                    ListItemComponent {
                        type: "header"
                        
                        Header {
                            title: formatDate(ListItemData)
                            subtitle: "Time to read:"
                            function formatDate(strDate) {
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
                            
                            contextActions: [
                                ActionSet {
                                    ActionItem {
                                        title: "Edit"
                                        imageSource: "asset:///images/menuicons/ic_edit.png"
                                        onTriggered: bookmark.ListItem.view.openEditSheet(ListItemData)
                                    }
                                    ActionItem {
                                        title: "Favorite"
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
                                                body: deletedItem ? "Bookmark deleted: " + deletedItem.title : ""
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
                                layout: StackLayout {
                                    orientation: LayoutOrientation.LeftToRight
                                }
                                
                                Container {
                                    minWidth: 100
                                    maxWidth: 100 
                                    maxHeight: rowHandler.layoutFrame.height
                                    
                                    ImageView {
                                        imageSource: ListItemData.image ? "file://" + ListItemData.image : "asset:///images/backpack.png"
                                        scalingMethod: ScalingMethod.AspectFill
                                        verticalAlignment: VerticalAlignment.Fill
                                        horizontalAlignment: HorizontalAlignment.Fill
                                    }                        
                                }
                                
                                Container {
                                    topPadding: 4
                                    rightPadding: 12
                                    bottomPadding: 12
                                    leftPadding: 14
                                    
                                    attachedObjects: LayoutUpdateHandler {
                                        id: rowHandler
                                    }
                                    
                                    Container {
                                        layout: StackLayout {
                                            orientation: LayoutOrientation.LeftToRight
                                        }
                                        leftPadding: 6
                                        
                                        Container {
                                            id: activity
                                            visible: !ListItemData.title
                                            topPadding: 15
                                            leftPadding: 4
                                            rightPadding: 12
                                            scaleX: 1.3
                                            scaleY: 1.3
                                            ActivityIndicator {
                                                running: true
                                            }
                                        }
                                        
                                        Label {
                                            id: bookmarkTitle
                                            text: ListItemData.title
                                            textStyle.fontSize: FontSize.XLarge
                                            translationX: -7
                                        }
                                    }
                                    
                                    Container {
                                        layout: DockLayout {}
                                        minWidth: Qt.pageHandler.layoutFrame.width - 125
                                        
                                        Container {
                                            layout: StackLayout {
                                                orientation: LayoutOrientation.LeftToRight
                                            }
                                            maxWidth: rowHandler.layoutFrame.width - timeHandler.layoutFrame.width - 20
                                            
                                            Container {
                                                id: iconContainer
                                                visible: ListItemData.favicon
                                                topPadding: 10
                                                rightMargin: 10
                                                
                                                ImageView {
                                                    id: iconImage
                                                    imageSource: "file://" + ListItemData.favicon
                                                    minWidth: 26
                                                    minHeight: 26
                                                }                            
                                            }
                                            
                                            Label {
                                                text: ListItemData.url
                                                textStyle.color: Color.Gray
                                                textStyle.fontSize: FontSize.Medium
                                                bottomMargin: 0
                                                leftMargin: 0
                                            }
                                        }
                                        
                                        Label {
                                            visible: ListItemData.type != 3 && ListItemData.type != 4
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
                                            attachedObjects: LayoutUpdateHandler {
                                                id: timeHandler
                                            }
                                        }
                                    }
                                    
                                    Container {
                                        visible: memoText.text.toString().length > 0
                                        
                                        Label {
                                            id: memoText
                                            text: ListItemData.memo
                                            multiline: true
                                            textStyle.color: Color.White
                                            textStyle.fontSize: FontSize.Medium
                                        }
                                    }
                                }
                            }                        
                            
                            ImageView {
                                imageSource: "asset:///images/keep.png"
                                visible: ListItemData.keep
                                horizontalAlignment: HorizontalAlignment.Right
                                translationX: -10
                                translationY: 12
                            }
                            
                            Divider {}
                            
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
            
            Container {
                id: emptyLabelContainer
                visible: false
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 90
                
                Label {
                    id: emptyLabel
                    horizontalAlignment: HorizontalAlignment.Center
                    textStyle.fontSize: FontSize.Large
                }
            }
        }
        
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            topPadding: 10
            rightPadding: 15
            bottomPadding: 10
            leftPadding: 15
            
            Container {                
                attachedObjects: LayoutUpdateHandler {
                    id: searchFormHandler
                }
                
                TextField {
                    id: query
                    hintText: "Search"
                    verticalAlignment: VerticalAlignment.Center
                    input.submitKey: SubmitKey.Search
                    input.onSubmitted: app.refreshBookmarks(query.text.trim())
                    onTextChanging: app.refreshBookmarks(query.text.trim())
                }
            }
            
            DropDown {
                id: filterType
                objectName: "filterType"
                preferredWidth: pageHandler.layoutFrame.width / 2
                onExpandedChanged: bookmarksList.opacity = 1 - 0.75 * expanded
                onSelectedValueChanged: app.refreshBookmarks(query.text.trim())
                
                options: [
                    Option {
                        text: "All items"
                        selected: true
                        value: 0
                    },
                    Option {
                        text: "Favorites"
                        value: 1
                    },
                    Option {
                        text: "Only articles"
                        value: 2
                    },
                    Option {
                        text: "Only videos"
                        value: 3
                    },
                    Option {
                        text: "Only images"
                        value: 4
                    }
                ]
            }
        }
    }
}
