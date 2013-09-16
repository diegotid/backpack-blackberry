
import bb.cascades 1.0

Page {
    id: browsePage
    objectName: "browsePage"
    
    titleBar: TitleBar {
        title: "My Backpack"
    }
    
    onCreationCompleted: loadBackground()
    
    function loadBackground() {
        backgroundRed.opacity = app.getBackgroundColour("red");
        backgroundGreen.opacity = app.getBackgroundColour("green");
        backgroundBlue.opacity = app.getBackgroundColour("blue");
        backgroundBase.opacity = app.getBackgroundColour("base");
    }
    
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
            
//		        dataModel: XmlDataModel {
//                    source: "debug.xml"
//                }
            
            onDataModelChanged: {
                oldestLabel.text = app.getOldestDate()
                quickestLabel.text = app.getQuickestSize()
            }
            
            listItemComponents: [
                
                ListItemComponent {
                    type: "header"
                    
                    Header {
//                            title: ListItemData.title // For debub.xml
                        title: ListItemData
                        property string dateTitle
                        
                        onTitleChanged: if (title != dateTitle) formatDate()
                        onDateTitleChanged: if (title != dateTitle) title = dateTitle
                        
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
                                    onTriggered: bookmark.ListItem.view.pushEditPage(ListItemData)
                                }
                                ActionItem {
                                    title: "Toggle keep after read"
                                    imageSource: "asset:///images/menuicons/zip.png"
                                    onTriggered: bookmark.ListItem.view.toggleKeep(ListItemData.keep == "false", ListItemData.id)   
                                }
                                DeleteActionItem {
                                    onTriggered: bookmark.ListItem.view.deleteBookmark(ListItemData.id)
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
                            leftPadding: 10
                            
                            Container {
                                visible: ListItemData.title ? false : true
                                
                                topPadding: 20
                                bottomPadding: 20
                                leftPadding: 20
                                rightPadding: 20
                                
                                ActivityIndicator {
                                    running: true
                                    scaleX: 1.3
                                    scaleY: 1.3
                                }
                            }
                        }
                        
                        Container {
                            topPadding: 8
                            bottomPadding: 12
                            rightPadding: 8
                            leftPadding: 18
                            
                            Container {
                                layout: AbsoluteLayout {}
                                
                                Label {
                                    id: bookmarkTitle
                                    text: ListItemData.title
                                    textStyle.fontSize: FontSize.XLarge
                                    preferredWidth: 700
                                }
                            }
                            
                            Container {
                                layout: AbsoluteLayout {}
                                
                                Label {
                                    text: ListItemData.memo ? ListItemData.memo : ListItemData.url
                                    textStyle.color: ListItemData.memo ? Color.create("#07b1e6") : Color.Gray
                                    textStyle.fontSize: FontSize.Medium
                                    multiline: ListItemData.memo
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
                            else if (event.touchType == TouchType.Up || event.touchType == TouchType.Cancel)
                                highlight.visible = false
                        }
                    }                        
                }
            ]
            
            onTriggered: {
                var selectedItem = dataModel.data(indexPath)
                invokeQuery.query.uri = selectedItem.url
                invokeQuery.query.updateQuery()
                app.removeBookmark(selectedItem.id)
                if (app.getSize() == 0) {
                    homePage.remove(browsePage);
                }
            }
            
            property Page editPage
            function pushEditPage(row) {
                if (!editPage) {
                    editPage = editPageDefinition.createObject();
                }
                editPage.item = row;
                homePage.backButtonsVisible = false
                homePage.push(editPage);
            }
            
            function deleteBookmark(id) {	                
                app.removeBookmark(id, true);
                if (app.getSize() == 0)
                    homePage.remove(browsePage);
            }
            
            function toggleKeep(keep, id) {
                app.keepBookmark(keep, id);
            }     
            
            attachedObjects: [
                Invocation {
                    id: invokeQuery
                    query.invokeTargetId: "sys.browser"
                    onArmed: trigger("bb.action.OPEN")
                },
                ComponentDefinition {
                    id: editPageDefinition
                    source: "InvokedForm.qml"
                }
            ]
        }
	}
}