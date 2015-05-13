
import bb.cascades 1.4
import bb.system 1.0
  
Page {
    id: browseListPage

    property bool filterExpand: false
    
    actionBarVisibility: ChromeVisibility.Overlay
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll

    keyListeners: [
        KeyListener {
            onKeyReleased: {
                searchForm.visible = true
                if (!query.focused) {
                    query.text = query.text + event.unicode
                    query.requestFocus()
                }
            }
        }   
    ]

    titleBar: TitleBar {
        id: hiddenTitleBar
        scrollBehavior: TitleBarScrollBehavior.Sticky
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: DockLayout {}
                FreeTitleBar {
                    id: freeTitleBar
                    visible: !searchForm.visible
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Fill
                }
                Container {
                    id: searchForm
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Fill
                    visible: false
                    leftPadding: 10.0
                    rightPadding: 10.0
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Button {
                        id: filterButton
                        imageSource: "asset:///images/menuicons/ic_overflow_tab.png"
                        preferredWidth: ui.sdu(1)
                        appearance: ControlAppearance.Plain
                        onClicked: filterExpand = !filterExpand
                    }
                    TextField {
                        id: query
                        hintText: "Search"
                        verticalAlignment: VerticalAlignment.Center
                        input.submitKey: SubmitKey.Search
                        input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Lose
                        onTextChanging: app.refreshBookmarks(query.text.trim())
                        onFocusedChanged: {
                            if (focused) {
                                filterExpand = false
                            }   
                        }
                    }
                    Button {
                        text: "Cancel"
                        appearance: ControlAppearance.Plain
                        horizontalAlignment: HorizontalAlignment.Right
                        preferredWidth: ui.sdu(20)
                        onClicked: {
                            filterExpand = false
                            searchForm.visible = false   
                            query.text = ''
                            filterType.selectedOption = filterType.options[0]
                            app.refreshBookmarks()
                        }
                    }
                }
            }
            expandableArea {
                indicatorVisibility: TitleBarExpandableAreaIndicatorVisibility.Hidden
                expanded: filterExpand
                content: RadioGroup {
                    id: filterType
                    objectName: "filterType"
                    onSelectedOptionChanged: app.refreshBookmarks(query.text.trim())
                    options: [
                        Option {
                            value: 0
                            text: "All items"
                            selected: true
                            onSelectedChanged: if (selected) filterButton.imageSource = "asset:///images/menuicons/ic_overflow_tab.png"
                        },
                        Option {
                            value: 1
                            text: "Favorites"
                            onSelectedChanged: if (selected) filterButton.imageSource = "asset:///images/menuicons/zip.png"
                        },
                        Option {
                            value: 2
                            text: "Only articles"
                            onSelectedChanged: if (selected) filterButton.imageSource = "asset:///images/menuicons/ic_doctype_generic.png"
                        },
                        Option {
                            value: 3
                            text: "Only videos"
                            onSelectedChanged: if (selected) filterButton.imageSource = "asset:///images/menuicons/ic_play.png"
                        },
                        Option {
                            value: 4
                            text: "Only images"
                            onSelectedChanged: if (selected) filterButton.imageSource = "asset:///images/menuicons/ic_view_image.png"
                        }
                    ]
                }
            }
        }
    }
    
    actions: [
        ActionItem {
            title: "Shuffle"
            ActionBar.placement: ActionBarPlacement.Signature
            imageSource: "asset:///images/buttons/shuffle.png"
            onTriggered: app.shuffleBookmark()
        },
        ActionItem {
            title: "Lounge"
            imageSource: "asset:///images/buttons/lounge.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                browseDialog.readmode = "Lounge"
                browseSheet.open()
            }
        },
        ActionItem {
            title: "Quickest"
            imageSource: "asset:///images/buttons/quickest.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                browseDialog.readmode = "Quickest"
                browseSheet.open()
            }
        },
        ActionItem {
            title: "Newest"
            imageSource: "asset:///images/menuicons/ic_to_top.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: bookmarks.scrollToPosition(ScrollPosition.Beginning, ScrollAnimation.Smooth)
        },
        ActionItem {
            title: "Oldest"
            imageSource: "asset:///images/menuicons/ic_to_bottom.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: bookmarks.scrollToPosition(ScrollPosition.End, ScrollAnimation.Smooth)
        },
        ActionItem {
            title: "Search"
            imageSource: "asset:///images/menuicons/ic_search.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {    
                searchForm.visible = true
            }
        }
    ]
    
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
                onClose: bookmarkSheet.close()
            }
        },
        Sheet {
            id: browseSheet
            BrowseDialog {
                id: browseDialog
                onClose: browseSheet.close()
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
            
            ListView {
                id: bookmarks
                objectName: "bookmarks"
                
                layout: StackListLayout {
                    headerMode: ListHeaderMode.Sticky
                }
                scrollRole: ScrollRole.Main
                
//                dataModel: XmlDataModel {
//                    source: "debug.xml"
//                }
                
                property int size;
                onSizeChanged: {
                	emptyLabelContainer.visible = (size == 0)
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
                
                listItemComponents: [
                    
                    ListItemComponent {
                        type: "header"
                        
                        Container {
                            
                            Container {
                                layout: DockLayout {}
                                preferredWidth: Qt.pageHandler.layoutFrame.width
                                horizontalAlignment: HorizontalAlignment.Fill
                                verticalAlignment: VerticalAlignment.Bottom
                                background: Color.create("#a0000000")
                                leftPadding: 12
                                rightPadding: 8
                                bottomPadding: 4
                                Label {
                                    text: formatDate(ListItemData)
//                                    textStyle.color: ui.palette.primary
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
                                Label {
                                    text: "Time to read:"
                                    textStyle.color: ui.palette.primary
                                    textStyle.fontSize: FontSize.XXSmall
                                    horizontalAlignment: HorizontalAlignment.Right
                                    verticalAlignment: VerticalAlignment.Bottom
                                }
                            }
                            
                            Container {
                                topMargin: 0
                                preferredHeight: 1
                                preferredWidth: Qt.pageHandler.layoutFrame.width
                                background: ui.palette.primary
                                opacity: 0.5
                            }               
                        }                             
                    },
                    
                    ListItemComponent {
                        type: "item"
                        
                        Container {
                            id: bookmark
                            layout: DockLayout {}
                            
                            attachedObjects: [
                                LayoutUpdateHandler {
                                    function fetchItemContent() {
                                        if (!ListItemData.fetched
                                        && (!ListItemData.image || ListItemData.image.toString().length == 0 // for '.'  meaning no image available
                                        || !ListItemData.favicon || ListItemData.favicon.toString().length == 0)) {
                                            bookmark.ListItem.view.fetchContent(ListItemData.url)
                                        }
                                    }
                                    onLayoutFrameChanged: fetchItemContent()
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
                                        title: "Favorite"
                                        imageSource: "asset:///images/menuicons/zip.png"
                                        onTriggered: bookmark.ListItem.view.toggleKeep(ListItemData.url, ListItemData.keep != true)
                                    }
                                    DeleteActionItem {
                                        onTriggered: {
                                            bookmark.ListItem.view.dataModel.remove(ListItemData)
                                            if (bookmark.ListItem.view.dataModel.isEmpty()) {
                                                bookmark.ListItem.view.setContentTabsEnabled(false);
                                            }
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
                                                        bookmark.ListItem.view.setContentTabsEnabled(true);
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
                                    layout: DockLayout {}
                                    minWidth: ui.sdu(12)
                                    maxWidth: ui.sdu(12) 
                                    maxHeight: rowHandler.layoutFrame.height

                                    ImageView {
                                        imageSource: ListItemData.image && ListItemData.image.toString().length > 1 ? "file://" + ListItemData.image : "asset:///images/backpack.png" // length > 1 is for '.'  meaning no image available 
                                        scalingMethod: ScalingMethod.AspectFill
                                        verticalAlignment: VerticalAlignment.Fill
                                        horizontalAlignment: HorizontalAlignment.Fill
                                    }
                                    
                                    Container {
                                        visible: !ListItemData.image || ListItemData.image.toString().length <= 1 // length > 1 is for '.'  meaning no image available
                                        background: Color.create(0, 0, 0, 0.5)
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
                                                accessibility.name: "Loading"
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
                                        minWidth: Qt.pageHandler.layoutFrame.width - ui.sdu(14)
                                        
                                        Container {
                                            layout: StackLayout {
                                                orientation: LayoutOrientation.LeftToRight
                                            }
                                            maxWidth: rowHandler.layoutFrame.width - timeHandler.layoutFrame.width - ui.sdu(3)
                                            
                                            Container {
                                                id: iconContainer
                                                visible: ListItemData.favicon && ListItemData.favicon.toString().length > 0
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
                                                text: bookmark.ListItem.view.domain(ListItemData.url)
                                                textStyle.color: Color.Gray
                                                textStyle.fontSize: FontSize.Medium
                                                bottomMargin: 0
                                                leftMargin: 0
                                            }
                                        }
                                        
                                        Label {
                                            visible: ListItemData.type != 3 && ListItemData.type != 4
                                            textStyle.color: ui.palette.primary
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
                
                function domain(url) {
                    var domain = url.substring(url.indexOf("://") + (url.indexOf("://") < 0 ? 1 : 3));
                    if (domain.indexOf('/') < domain.indexOf('.')) {
                        return domain
                    } else {
                        return domain.substring(0, domain.indexOf('/'))
                    }
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
                
                function fetchContent(url) {
                    app.fetchContent(url)
                }
                
                function setContentTabsEnabled(enabled) {
                    browseListPage.parent.setEnabled(enabled);
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
    }
}
