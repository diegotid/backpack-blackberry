
import bb.cascades 1.3
import bb.system 1.0

Page {
    objectName: "readPage"
    
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            onTriggered: articlesPane.pop()
        }
    }        
    actionBarVisibility: ChromeVisibility.Overlay
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    
    property string link
    property string hash
    property bool isFavourite: false

    onLinkChanged: putBackAction.enabled = link.length > 0 && !isFavourite && !app.getKeepAfterRead()

    function reset() {
        link = ""
        host.resetText()
        articleBody.reset()
        isFavourite = false
        toggleFavourite.reset()
        articleDate.resetText()
        putBackAction.enabled = false
        titleBar.title = "Loading article..."
    }
    
    attachedObjects: [
        SystemToast {
            id: stayToast
            body: "Article will stay in your Backpack"
        }
    ]
    
    actions: [
        ActionItem {
            title: "View source"
            imageSource: "asset:///images/menuicons/ic_browser.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            enabled: link.length > 0
            onTriggered: {
                app.browseBookmark(link)
                app.logEvent("Browse")
            }
        },
        ActionItem {
            title: "Favorite"
            id: toggleFavourite
            imageSource: "asset:///images/menuicons/zip.png"
            ActionBar.placement: ActionBarPlacement.Signature
            function reset() {
                toggleFavourite.title = "Favorite"
                toggleFavourite.imageSource = "asset:///images/menuicons/zip.png"
            }
            onTriggered: {
                app.add(link)
                isFavourite = !isFavourite
                app.keepBookmark(link, isFavourite)
                if (isFavourite && !app.getKeepAfterRead()) {
                    stayToast.show()
                }
                putBackAction.enabled = false
            }
        },
        ActionItem {
            title: "Put back"
            id: putBackAction
            imageSource: "asset:///images/menuicons/ic_add.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            enabled: link.length > 0 && !isFavourite && !app.getKeepAfterRead()
            onTriggered: {
                app.add(link)
                putBackAction.enabled = false
                if (!app.getKeepAfterRead()) stayToast.show()
            }
        },
        ActionItem {
            title: "Update"
            imageSource: "asset:///images/menuicons/ic_reload.png"
            enabled: link.length > 0
            onTriggered: {
                var reloadLink = link
                reset()
                app.pocketDownload(reloadLink)
            }
        },
        InvokeActionItem {
            title: "Share"
            query {
                mimeType: "text/plain"
                invokeActionId: "bb.action.SHARE"
            }
            onTriggered: data = link
        }
    ]

    titleBar: TitleBar {
        id: titleBar
        scrollBehavior: TitleBarScrollBehavior.NonSticky
        title: "Loading article..."
        kind: TitleBarKind.FreeForm
        visibility: ChromeVisibility.Compact
        function reset() {
            articleTitle = "Loading article..."
        }
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: DockLayout {}
                rightPadding: ui.sdu(2)
                leftPadding: ui.sdu(1)
                Label {
                    id: articleTitle
                    text: titleBar.title
                    visible: text == "Loading article..."
                    verticalAlignment: VerticalAlignment.Center
                }
                ActivityIndicator {
                    running: titleBar.title == "Loading article..."
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                }
                ImageView {
                    id: favIndicator
                    objectName: "favIndicator"
                    visible: isFavourite
                    imageSource: "asset:///images/keep.png"
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    onVisibleChanged: {
                        toggleFavourite.imageSource = "asset:///images/menuicons/" + (visible ? "unfav" : "zip") + ".png"
                        toggleFavourite.title = visible ? "Unfavorite" : "Favorite"
                    }
                }
                Container {
                    visible: host.text.length > 0
                    verticalAlignment: VerticalAlignment.Center
                    Label {
                        id: host
                        objectName: "headerHost"
                        textStyle.fontSize: FontSize.Small
                        bottomMargin: 0
                    }
                    Label {
                        id: articleDate
                        objectName: "headerDate"
                        textStyle.fontSize: FontSize.XXSmall
                        topMargin: 0
                    }
                }
            }
        }
    }

    Container {
        background: Color.create("#f3e1c4")

        attachedObjects: LayoutUpdateHandler {
            id: pageHandler
        }
        
        ScrollView {
            scrollViewProperties {
                scrollMode: ScrollMode.Vertical
            }
            horizontalAlignment: HorizontalAlignment.Fill

            WebView {
                id: articleBody
                objectName: "articleBody"
                property string style: app.getReadStyle()
                property string initial: "<html><head>" + style + "</head><body></body></html>" 
                html: initial
                property string body
                onBodyChanged: html = "<html><head>" + style + "</head><body>" + (body.length > 0 ? "<h1>" + titleBar.title + "</h1>" : "") + body + "</body>"
                function reset() { body = "" }
            }
        }
    }
}