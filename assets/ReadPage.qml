
import bb.cascades 1.4

Page {
    objectName: "readPage"
    
    signal close()
    
    actionBarVisibility: ChromeVisibility.Overlay
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll

    titleBar: TitleBar {
        id: titleBar
        scrollBehavior: TitleBarScrollBehavior.NonSticky
        title: "Loading article..."
        kind: TitleBarKind.FreeForm
        visibility: ChromeVisibility.Compact
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: DockLayout {}
                rightPadding: ui.sdu(2)
                leftPadding: ui.sdu(1)
                Label {
                    text: titleBar.title
                    visible: text == "Loading article..."
                    verticalAlignment: VerticalAlignment.Center
                }
                ActivityIndicator {
                    running: titleBar.title == "Loading article..."
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
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
                        objectName: "headerDate"
                        textStyle.fontSize: FontSize.XXSmall
                        topMargin: 0
                    }
                }
            }
        }
    }

    Container {
        background: Color.White
        
        ScrollView {
            scrollViewProperties {
                scrollMode: ScrollMode.Vertical
            }
            
            WebView {
                objectName: "articleBody"
                property string body
                property string style: "<style type='text/css'>p, img { float: left; clear: both; width: 100% }</style>"
                onBodyChanged: html = "<html><head>" + style + "</head><body><h1>" + titleBar.title + "</h1>" + body + "</body>"
            }
        }
    }
}