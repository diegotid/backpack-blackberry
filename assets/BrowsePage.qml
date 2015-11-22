
import bb.cascades 1.3
import bb.platform 1.2
import bb.system 1.0

NavigationPane {
    id: articlesPane
    objectName: "articlesPane"
    
    attachedObjects: [
        PaymentManager {
            id: payment
            windowGroupId: Application.mainWindow.groupId
            onPurchaseFinished: {
                if (reply.errorCode == 0) {
                    app.setPremium()
                    premiumDialog.body = "Purchase completed!"
                                            + "\n\nFeatures unlocked:"
                                            + "\n- Article search"
                                            + "\n- Reading specific articles"
                                            + "\n- Article skipping on Lounge & Quickest"
                    premiumDialog.cancelButton.label = "Check it!"
                    premiumDialog.confirmButton.label = undefined
                    premiumDialog.show()
                } else {
                    premiumDialog.body = "Purchase didn't complete: " + reply.errorText
                    premiumDialog.cancelButton.label = "Try later"
                    premiumDialog.confirmButton.label = "Try again now"
                    premiumDialog.show()
                }
            }
            onPriceFinished: {
                premiumDialog.body += ": " + reply.price
                premiumDialog.show()
            }
        }
    ]

    onCreationCompleted: {
        payment.setConnectionMode(0)
    }
  
    function startPurchase() {
        premiumDialog.body = "The following premium features are only available on the full version of Backpack:"
                                + "\n- Article search"
                                + "\n- Reading specific articles"
                                + "\n- Article skipping on Lounge & Quickest"
                                + "\n\nOne-time single payment"
        premiumDialog.cancelButton.label = "Maybe later"
        premiumDialog.confirmButton.label = "Get it now!"
        premiumDialog.show()
        payment.requestPrice("", "SKU59983351")
    }
    
    function confirmPurchase() {
        payment.requestPurchase("", "SKU59983351")
    }

    property bool filterExpand: false

    property Page readPage
    function getReadPage() {
        if (!readPage) {
            readPage = readPageDefinition.createObject()
        }
        return readPage
    }    
    onPopTransitionEnded: readPage.destroy()
    
    function preparePopTransition() {
        if (readPage && browseDialog.offset > 0) {
            browseSheet.open()
        }
    }
    
    function updateUsername(username) {
        freeTitleBar.username = username
    }
    
    function updateProgress(progress) {
        freeTitleBar.progress = progress
    }

    function performRead(link) {
        var articlePage = getReadPage()
        articlePage.reset()
        articlesPane.push(articlePage)
        app.readBookmark(link)
        app.logEvent("Read")
    }
    
    function performShuffle() {
        var articlePage = getReadPage()
        articlePage.reset()
        articlesPane.push(articlePage)
        app.shuffleBookmark()
        app.logEvent("Shuffle")
    }
    
    function performLounge() {
        browseDialog.readmode = "Lounge"
        browseDialog.username = freeTitleBar.username
        browseDialog.keepAfterRead = app.getKeepAfterRead()
        browseDialog.ignoreFavourites = app.getIgnoreKeptLounge()
        browseSheet.open()
    }
    
    function performQuickest() {
        browseDialog.readmode = "Quickest"
        browseDialog.username = freeTitleBar.username
        browseDialog.keepAfterRead = app.getKeepAfterRead()
        browseDialog.ignoreFavourites = app.getIgnoreKeptQuickest()
        browseSheet.open()
    }

    Page {
        id: browseListPage
        objectName: "browseListPage"

        actionBarVisibility: ChromeVisibility.Overlay
        actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
        
        property int listSize
        onListSizeChanged: updateEmptyLabel()

        function updateEmptyLabel() {    
            var tale = query.text.toString().trim().length > 0 ? " found" : ""
            switch (filterType.selectedOption.value) {
                case 1:
                    emptySearchHint.text = "No favorited articles" + tale
                    break
                case 2:
                    emptySearchHint.text = "No articles" + tale
                    break
                case 3:
                    emptySearchHint.text = "No videos" + tale
                    break
                case 4:
                    emptySearchHint.text = "No images" + tale
                    break
                default:
                    emptySearchHint.text = tale.toString().length > 0 ? "No articles found" : "Nothing in your Backpack"
            }
        }
        
        keyListeners: [
            KeyListener {
                onKeyReleased: {
                    if (app.isPremium()) {
                        searchForm.visible = true
                        if (!query.focused) {
                            query.text = query.text + event.unicode
                            query.requestFocus()
                        }
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
                        verticalAlignment: VerticalAlignment.Fill
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
                            onTextChanging: {
                                app.refreshBookmarks(query.text.trim())
                            }
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
                                query.text = ""
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
                        onSelectedOptionChanged: {
                            app.refreshBookmarks(query.text.trim())
                            filterExpand = false
                        }
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
                onTriggered: {
                    if (app.getSettingsUnderstood() || app.getKeepAfterRead()) {
                        performShuffle()
                    } else {
                        discardDialog.mode = "Shuffle"
                        discardDialog.show()
                    }
                }
                enabled: browseListPage.listSize > 0
            },
            ActionItem {
                title: "Lounge"
                imageSource: "asset:///images/buttons/lounge.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                onTriggered: {
                    if (app.getSettingsUnderstood() || app.getKeepAfterRead()) {
                        performLounge()
                    } else {
                        discardDialog.mode = "Lounge"
                        discardDialog.show()
                    }
                }
                enabled: browseListPage.listSize > 0
            },
            ActionItem {
                title: "Quickest"
                imageSource: "asset:///images/buttons/quickest.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                onTriggered: {
                    if (app.getSettingsUnderstood() || app.getKeepAfterRead()) {
                        performQuickest()
                    } else {
                        discardDialog.mode = "Quickest"
                        discardDialog.show()
                    }
                }
                enabled: browseListPage.listSize > 0
            },
            ActionItem {
                title: "Newest"
                imageSource: "asset:///images/menuicons/ic_to_top.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                onTriggered: bookmarks.scrollToPosition(ScrollPosition.Beginning, ScrollAnimation.Smooth)
                enabled: browseListPage.listSize > 0
            },
            ActionItem {
                title: "Oldest"
                imageSource: "asset:///images/menuicons/ic_to_bottom.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                onTriggered: bookmarks.scrollToPosition(ScrollPosition.End, ScrollAnimation.Smooth)
                enabled: browseListPage.listSize > 0
            },
            ActionItem {
                title: "Search"
                imageSource: "asset:///images/menuicons/ic_search.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                enabled: browseListPage.listSize > 0
                onTriggered: {    
                    if (app.isPremium()) {
                        searchForm.visible = true
                    } else {
                        startPurchase()
                    }
                }
            },
            ActionItem {
                title: "Sync Pocket now"
                imageSource: "asset:///images/menuicons/ic_reload.png"
                ActionBar.placement: ActionBarPlacement.OnBar
                enabled: freeTitleBar.username.length > 0
                onTriggered: {
                    app.pocketRetrieve()
                }
            }
        ]
        
        attachedObjects: [
            ComponentDefinition {
                id: readPageDefinition
                source: "ReadPage.qml"
            },
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
                    objectName: "browseDialog"
                    onClose: browseSheet.close()
                }
            },
            SystemDialog {
                id: discardDialog
                title: "Persistence options"
                body: "Under current settings non-favorited articles are removed after read to keep your Backpack organized"
                customButton.label: "Settings"
                confirmButton.label: "Understood"
                property string mode
                property string link
                onFinished: {
                    if (result == SystemUiResult.ConfirmButtonSelection) {
                        app.setSettingsUnderstood()
                        switch (mode) {
                            case "Read":
                                performRead(link)
                                break
                            case "Shuffle":
                                performShuffle()
                                break
                            case "Lounge":
                                performLounge()
                                break
                            case "Quickest":
                                performQuickest()
                                break
                        }
                    } else if (result == SystemUiResult.CustomButtonSelection) {
                        settingsSheet.open()
                    }
                }
            },
            SystemDialog {
                id: premiumDialog
                title: "Get Backpack full version"
                onFinished: {
                    if (result == SystemUiResult.ConfirmButtonSelection) {
                        confirmPurchase()
                    }
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
                    
/*                dataModel: XmlDataModel {
                        source: "debug.xml"
                    }
*/                
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
                                            title: ListItemData.keep == "true" ? "Unfavorite" : "Favorite"
                                            imageSource: ListItemData.keep == "true" ? "asset:///images/menuicons/unfav.png" : "asset:///images/menuicons/zip.png"
                                            onTriggered: bookmark.ListItem.view.toggleKeep(ListItemData.url, ListItemData.keep != "true")
                                        }
                                        DeleteActionItem {
                                            onTriggered: {
                                                bookmark.ListItem.view.dataModel.remove(ListItemData)
                                                bookmark.ListItem.view.updateListSize()
                                                deletedBookmark.deletedItem = ListItemData
                                                deletedBookmark.show()
                                            }                                   
                                            attachedObjects: [
                                                SystemToast {
                                                    id: deletedBookmark
                                                    body: deletedItem ? "Article deleted: " + deletedItem.title : ""
                                                    button.label: "Undo"
                                                    button.enabled: true 
                                                    property variant deletedItem
                                                    onFinished: {
                                                        if (result == SystemUiResult.ButtonSelection) {
                                                            bookmark.ListItem.view.dataModel.insert(deletedItem)
                                                            bookmark.ListItem.view.updateListSize()
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
                                            imageSource: (ListItemData.image && ListItemData.image.toString().length > 1)
                                                ? "file://" + ListItemData.image // length > 1 is for '.'  meaning no image available
                                                : (ListItemData.first_img ? "file://" + ListItemData.first_img : "asset:///images/backpack.png") 
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
                                                    accessibility.name: "Loading"
                                                }
                                            }
                                            
                                            Label {
                                                id: bookmarkTitle
                                                text: ListItemData.title
                                                textStyle.fontSize: FontSize.XLarge
                                                maxWidth: rowHandler.layoutFrame.width - (ListItemData.keep == "true" ? ui.sdu(8) : 0)
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
                                                    verticalAlignment: VerticalAlignment.Center
                                                    
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
                                    visible: ListItemData.keep == "true"
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
                        if (app.isPremium()) {
                            var selectedItem = dataModel.data(indexPath)
                            if (selectedItem.keep == "true"
                            || app.getSettingsUnderstood()
                            || app.getKeepAfterRead()) {
                                performRead(selectedItem.url)
                            } else {
                                discardDialog.mode = "Read"
                                discardDialog.link = selectedItem.url
                                discardDialog.show()
                            }
                        } else {
                            startPurchase()
                        }
                    }
                    
                    function domain(url) {
                        var domain = url.substring(url.indexOf("://") + (url.indexOf("://") < 0 ? 1 : 3));
                        if (domain.indexOf('/') > domain.indexOf('.')) {
                            domain = domain.substring(0, domain.indexOf('/'))
                        }
                        if (domain.indexOf("www.") == 0) {
                            domain = domain.substring(4)
                        }
                        return domain
                    }
                    
                    function openEditSheet(row) {
                        invokedForm.item = row
                        invokedForm.username = freeTitleBar.username
                        bookmarkSheet.open()
                    }
                    
                    function deleteBookmark(ListItemData) {
                        app.removeBookmark(ListItemData.url);
                    }
                    
                    function toggleKeep(url, keep) {
                        app.keepBookmark(url, keep)
                    }
                    
                    function fetchContent(url) {
                        app.fetchContent(url)
                    }
                    
                    function updateListSize(diff) {
                        browseListPage.listSize = bookmarks.dataModel.childCount([])
                    }
                }
                
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    topPadding: 90
                    
                    Label {
                        id: emptySearchHint
                        visible: browseListPage.listSize == 0
                        horizontalAlignment: HorizontalAlignment.Center
                        textStyle.fontSize: FontSize.Large
                    }
                }
            }
        }
    }
}