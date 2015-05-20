
import bb.cascades 1.4

Page {
    signal close()
    
    property variant bookmark
    property variant next
    property int offset
    property int minSize: 10000
    
    property string readmode: "Backpack"
    property string username
    property int keepAfterRead
    property bool ignoreFavourites

    onReadmodeChanged: {
        offset = 0
        nextButton.enabled = true
        switch (readmode) {
            case "Quickest":
                do {
                    bookmark = app.quickestBookmark(offset++)
                } while (bookmark.size < minSize)
                break
            case "Lounge":
                bookmark = app.loungeBookmark(offset++)
                break
        }
    }
    
    titleBar: TitleBar {
        title: readmode + " reading"
        
        dismissAction: ActionItem {
            title: "Cancel"            
            onTriggered: {
                readmode = "Backpack"
                browseDialog.close()
            }
        }
        
        acceptAction: ActionItem {
            title: "Next"
            id: nextButton
            onTriggered: {
                switch (readmode) {
                    case "Quickest":
                        next = app.quickestBookmark(offset++)
                        break
                    case "Lounge":
                        next = app.loungeBookmark(offset++)
                        break
                }
                if (next == null || next.size == null || next.size < minSize) {
                    enabled = false
                } else {
                    bookmark = next
                }
            }

        }
    }
    
    onBookmarkChanged: {
        favourite.visible = bookmark.keep
        articleTitle.text = bookmark.title
        articleSize.text = formatTime(bookmark.size)
        articleImage.imageSource = (bookmark.image && bookmark.image.toString().length > 1) ? "file://" + bookmark.image : "asset:///images/backpack.png" // length > 1 is for '.'  meaning no image available
        articleIcon.imageSource = "file://" + bookmark.favicon
        articleUrl.text = domain(bookmark.url)
    }
    
    function domain(url) {
        var domain = url.substring(url.indexOf("://") + (url.indexOf("://") < 0 ? 1 : 3));
        if (domain.indexOf('/') < domain.indexOf('.')) {
            return domain
        } else {
            return domain.substring(0, domain.indexOf('/'))
        }
    }

    function formatTime(size) {
        if (size.toString().indexOf("min") < 0) {
            if (size == 0)
                return "";
            var k10 = (size - size % minSize) / minSize
            switch (k10) {
                case 0: return "< 1 minute"
                case 1: return "1 minute"
                default: return k10 + " minutes" 
            }
        }
    }
    
    Container {
        layout: DockLayout {}
        
        attachedObjects: LayoutUpdateHandler {
            id: pageHandler
        }
        
        Container {
            layout: AbsoluteLayout {}
            verticalAlignment: VerticalAlignment.Bottom
            
            ImageView {
                id: articleImage
                imageSource: item ? "file://" + item.image : "asset:///images/backpack.png"
                scalingMethod: ScalingMethod.AspectFill
                verticalAlignment: VerticalAlignment.Fill
                horizontalAlignment: HorizontalAlignment.Fill
                minWidth: pageHandler.layoutFrame.width
                preferredHeight: pageHandler.layoutFrame.height
                opacity: 0.5
                
                attachedObjects: LayoutUpdateHandler {
                    id: imageHandler
                }
            }
            
            ImageView {
                imageSource: "asset:///images/shadow.png"
                scalingMethod: ScalingMethod.AspectFill
                verticalAlignment: VerticalAlignment.Fill
                horizontalAlignment: HorizontalAlignment.Fill
                minWidth: imageHandler.layoutFrame.width
                minHeight: imageHandler.layoutFrame.height
            }
        }

        Container {
            topPadding: ui.sdu(4)
            leftPadding: ui.sdu(4)
            rightPadding: ui.sdu(4)
            bottomPadding: ui.sdu(5)
            verticalAlignment: VerticalAlignment.Bottom
            horizontalAlignment: HorizontalAlignment.Fill
            
            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill

                Label {
                    id: articleSize
//                    text: "16 mins"
                    textStyle.color: ui.palette.primary
                    textStyle.fontSize: FontSize.Large
                }			
                
                ImageView {
                    id: favourite
                    imageSource: "asset:///images/keep.png"
                    maxWidth: ui.du(6)
                    maxHeight: ui.du(6)
                    scalingMethod: ScalingMethod.AspectFill
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    visible: false
                }        
            }
                                        
            Label {
                id: articleTitle
                multiline: true
//                text: "Título del artículo que ocupe muchas líneas y pelee espacio con el tiempo de lectura. Título del artículo que ocupe muchas líneas y pelee espacio con el tiempo de lectura. Título del artículo que ocupe muchas líneas y pelee espacio con el tiempo de lectura. "
//                text: "Título del artículo que ocupe poco"
                textStyle.color: Color.White
                textStyle.fontSize: FontSize.Large
                topMargin: ui.sdu(3)
            }
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                topMargin: ui.sdu(3)
                
                ImageView {
                    id: articleIcon
//                    imageSource: "asset:///images/favicon.png"
                    verticalAlignment: VerticalAlignment.Center
                    minWidth: 26
                    minHeight: 26
                }
                
                Label {
                    id: articleUrl
//                    text: "http://www.bbornot2b.com/"
                    textStyle.fontSize: FontSize.XSmall
                    verticalAlignment: VerticalAlignment.Center
                }
            }
            
            Button {
                text: "Read"
                horizontalAlignment: HorizontalAlignment.Fill
                topMargin: ui.sdu(4)
                onClicked: {
                    app.browseBookmark(bookmark.url)
                    browseDialog.close()
                }
            }
            
            Container {
                Label {
                    text: (ignoreFavourites || !keepAfterRead ? "Current settings: " : "")
                    + (ignoreFavourites ? "No favorites" : "")
                    + (ignoreFavourites && !keepAfterRead ? ", " : "")
                    + (keepAfterRead ? "" : (username.length > 0 ? "Archiving" : "Discarding") + " read articles")
                    multiline: true
                    textStyle.color: Color.LightGray
                    textStyle.fontSize: FontSize.XSmall
                }
            }
        }
    }
}