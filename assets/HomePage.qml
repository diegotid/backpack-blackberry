
import bb.cascades 1.0

Page {
    objectName: "homePage"
    
    property variant figures
    onFiguresChanged: {
        no_articles.setText(figures.articles)
        no_videos.setText(figures.videos)
        no_images.setText(figures.images)
    }
    
    property variant loungeItem
    onLoungeItemChanged: {
        if (loungeImage.image && loungeImage.image.toString().length > 0) {
            loungeImage.setImageSource("file://" + loungeItem.image)
        } else {
            loungeImage.setImageSource("asset:///images/lounge.png")
        } 
        loungeLabel.setText(loungeItem.label)
        loungeLabelZip.setVisible(loungeItem.keep && loungeItem.keep == "true")
        switch (loungeItem.type) {
            case 3: loungeLabelType.imageSource = "asset:///images/mini_video.png"; break; 
            case 4: loungeLabelType.imageSource = "asset:///images/mini_image.png"; break; 
            default: loungeLabelType.imageSource = "asset:///images/mini_article.png" 
        }
    }
    
    property variant oldestItem
    onOldestItemChanged: {
        if (oldestImage.image && oldestImage.image.toString().length > 0) {
            oldestImage.setImageSource("file://" + oldestItem.image)
        } else {
            oldestImage.setImageSource("asset:///images/oldest.png")
        } 
        oldestLabel.setText(oldestItem.label)
        oldestLabelZip.setVisible(oldestItem.keep && oldestItem.keep == "true")
        switch (oldestItem.type) {
            case 3: oldestLabelType.imageSource = "asset:///images/mini_video.png"; break; 
            case 4: oldestLabelType.imageSource = "asset:///images/mini_image.png"; break; 
            default: oldestLabelType.imageSource = "asset:///images/mini_article.png" 
        }
    }
    
    property variant quickestItem
    onQuickestItemChanged: {
        if (quickestItem.image && quickestItem.image.toString().length > 0) {
            quickestImage.setImageSource("file://" + quickestItem.image)
        } else {
            quickestImage.setImageSource("asset:///images/quickest.png")
        } 
        quickestLabel.setText(quickestItem.label)
        quickestLabelZip.setVisible(quickestItem.keep && quickestItem.keep == "true")
        switch (quickestItem.type) {
            case 3: quickestLabelType.imageSource = "asset:///images/mini_video.png"; break; 
            case 4: quickestLabelType.imageSource = "asset:///images/mini_image.png"; break; 
            default: quickestLabelType.imageSource = "asset:///images/mini_article.png" 
        }
    }
    
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
    
    Container {
        layout: DockLayout {}
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
        background: Color.create("#222222")

        attachedObjects: LayoutUpdateHandler {
            id: pageHandler
        }
        
        Container { // 1st row
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            horizontalAlignment: HorizontalAlignment.Fill
            
            Container {
                layout: DockLayout {}
                topPadding: 15
                rightPadding: 10
                bottomPadding: 10
                leftPadding: 15
                
                ImageView {
                    imageSource: "asset:///images/backpack.png"
                    scalingMethod: ScalingMethod.AspectFill
                    verticalAlignment: VerticalAlignment.Fill
                    horizontalAlignment: HorizontalAlignment.Fill
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 2 - 25
                    maxHeight: pageHandler.layoutFrame.height / 2 - 25
                }
                
                ImageView {
                    imageSource: "asset:///images/home-shadow.png"
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 3 - 25
                    maxHeight: pageHandler.layoutFrame.height / 3 - 25
                    verticalAlignment: VerticalAlignment.Bottom
                }
                                    
                Container {
                    layout: DockLayout {}
                    background: Color.create(0, 0, 0, .5)
                    horizontalAlignment: HorizontalAlignment.Fill
                    topPadding: 2
                    rightPadding: 15
                    bottomPadding: 8
                    leftPadding: 15

                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        horizontalAlignment: HorizontalAlignment.Center
                        
                        Label {
                            id: no_articles
                            textStyle.color: Color.White
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                            rightMargin: 0
                        }
                        ImageView {
                            imageSource: "asset:///images/mini_article.png"
                            verticalAlignment: VerticalAlignment.Center
                        }
                        
                        Label {
                            id: no_videos
                            textStyle.color: Color.White
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                            rightMargin: 0
                        }
                        ImageView {
                            imageSource: "asset:///images/mini_video.png"
                            verticalAlignment: VerticalAlignment.Center
                        }
                        
                        Label {
                            id: no_images
                            textStyle.color: Color.White
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                            rightMargin: 0
                        }
                        ImageView {
                            imageSource: "asset:///images/mini_image.png"
                            verticalAlignment: VerticalAlignment.Center
                        }
                    }
                }
                
				Container {
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 15
                    verticalAlignment: VerticalAlignment.Bottom
                    
                    Button {
                        text: "Shuffle"
                        imageSource: "asset:///images/buttons/shuffle.png"
                        onTouch: {
                            if (event.touchType == TouchType.Up) {
                                app.shuffleBookmark()
                            }
                        }
                    }                
                }
            }
            
            Container {
                layout: DockLayout {}
                topPadding: 15
                rightPadding: 15
                bottomPadding: 10
                leftPadding: 10
                
                ImageView {
                    id: oldestImage
                    imageSource: "asset:///images/oldest.png" 
                    scalingMethod: ScalingMethod.AspectFill
                    verticalAlignment: VerticalAlignment.Fill
                    horizontalAlignment: HorizontalAlignment.Fill
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 2 - 25
                    maxHeight: pageHandler.layoutFrame.height / 2 - 25
                }
                
                ImageView {
                    imageSource: "asset:///images/home-shadow.png"
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 3 - 25
                    maxHeight: pageHandler.layoutFrame.height / 3 - 25
                    verticalAlignment: VerticalAlignment.Bottom
                }
                
                Container {
                    layout: DockLayout {}
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    Container {
                        layout: DockLayout {}
                        background: Color.create(0, 0, 0, .5)
                        horizontalAlignment: HorizontalAlignment.Fill
                        topPadding: 2
                        rightPadding: 15
                        bottomPadding: 8
                        leftPadding: 15
                        
                        ImageView {
                            id: oldestLabelType
                            horizontalAlignment: HorizontalAlignment.Left
                            verticalAlignment: VerticalAlignment.Center
                        }
                        
                        ImageView {
                            id: oldestLabelZip
                            visible: false
                            imageSource: "asset:///images/mini_keep.png"
                            horizontalAlignment: HorizontalAlignment.Right
                            translationX: 3
                            translationY: 6
                        }
                        
                        Label {
                            horizontalAlignment: HorizontalAlignment.Center                          
                            id: oldestLabel
                            textStyle.color: Color.White
                            onCreationCompleted: formatDate()
                            onTextChanged: formatDate()
                            function formatDate() {
                                if (text.substring(4,5) == "-") {
                                    var today = new Date();
                                    today = new Date(today.getFullYear(), today.getMonth(), today.getDate(), 0, 0, 0, 0);
                                    var header = new Date(text.substring(0,4), text.substring(5,7) - 1, text.substring(8,10), 0, 0, 0, 0);
                                    var hours = (today.getTime() - header.getTime()) / 1000 / 60 / 60;
                                    var days = (hours - hours % 24) / 24; 
                                    switch (days) {
                                        case 0:
                                            text = "Today";
                                            break;
                                        case 1:
                                            text = "Yesterday";
                                            break;
                                        default:
                                            if (days < 7) {
                                                text = days + " day" + (days > 1 ? "s" : "") + " ago";
                                                break;   
                                            } else if (days < 31) {
                                                var weeks = (days - days % 7) / 7;
                                                text = weeks + " week" + (weeks > 1 ? "s" : "") + " ago";
                                                break;
                                            }
                                            var months = (days - days % 31) / 31;
                                            text = months + " month" + (months > 1 ? "s" : "") + " ago";
                                    }
                                }
                            }
                        }
                    }
                }
                
                Container {
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 15
                    verticalAlignment: VerticalAlignment.Bottom
                    
                    Button {    
                        text: "Oldest"
                        imageSource: "asset:///images/buttons/oldest.png"
                        onTouch: {
                            if (event.touchType == TouchType.Up) {
                                app.browseBookmark(oldestItem.url, text)
                            }
                        }
                    }                    
                }
            }
        }
        
        Container { // 2nd row
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            horizontalAlignment: HorizontalAlignment.Fill
            verticalAlignment: VerticalAlignment.Bottom
            
            Container {
                layout: DockLayout {}
                topPadding: 10
                rightPadding: 10
                bottomPadding: 15
                leftPadding: 15
                
                ImageView {
                    id: loungeImage
                    objectName: "loungeImage"
                    imageSource: "asset:///images/lounge.png"
                    scalingMethod: ScalingMethod.AspectFill
                    verticalAlignment: VerticalAlignment.Fill
                    horizontalAlignment: HorizontalAlignment.Fill
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 2 - 25
                    maxHeight: pageHandler.layoutFrame.height / 2 - 25
                }
                
                ImageView {
                    imageSource: "asset:///images/home-shadow.png"
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 3 - 25
                    maxHeight: pageHandler.layoutFrame.height / 3 - 25
                    verticalAlignment: VerticalAlignment.Bottom
                }
                
                Container {
                    layout: DockLayout {}
                    background: Color.create(0, 0, 0, .5)
                    horizontalAlignment: HorizontalAlignment.Fill
                    topPadding: 2
                    rightPadding: 15
                    bottomPadding: 8
                    leftPadding: 15
                    
                    ImageView {
                        id: loungeLabelType
                        horizontalAlignment: HorizontalAlignment.Left
                        verticalAlignment: VerticalAlignment.Center
                    }
                    
                    ImageView {
                        id: loungeLabelZip
                        visible: false
                        imageSource: "asset:///images/mini_keep.png"
                        horizontalAlignment: HorizontalAlignment.Right
                        translationX: 3
                        translationY: 6
                    }
                    
                    Label {
                        id: loungeLabel
                        textStyle.color: Color.White
                        horizontalAlignment: HorizontalAlignment.Center                          
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
                
                Container {
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 15
                    verticalAlignment: VerticalAlignment.Bottom
                    
                    Button {
                        text: "Lounge"
                        imageSource: "asset:///images/buttons/lounge.png"                        
                        onTouch: {
                            if (event.touchType == TouchType.Up) {
                                app.browseBookmark(loungeItem.url, text)
                            }
                        }
                    }                    
                }
            }
            
            Container {
                layout: DockLayout {}
                topPadding: 10
                rightPadding: 15
                bottomPadding: 15
                leftPadding: 10
                
                ImageView {
                    id: quickestImage
                    imageSource: "asset:///images/quickest.png" 
                    scalingMethod: ScalingMethod.AspectFill
                    verticalAlignment: VerticalAlignment.Fill
                    horizontalAlignment: HorizontalAlignment.Fill
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 2 - 25
                    maxHeight: pageHandler.layoutFrame.height / 2 - 25
                }
                
                ImageView {
                    imageSource: "asset:///images/home-shadow.png"
                    minWidth: pageHandler.layoutFrame.width / 2 - 25
                    maxWidth: pageHandler.layoutFrame.width / 2 - 25
                    minHeight: pageHandler.layoutFrame.height / 3 - 25
                    maxHeight: pageHandler.layoutFrame.height / 3 - 25
                    verticalAlignment: VerticalAlignment.Bottom
                }
                                    
                Container {
                    layout: DockLayout {}
                    background: Color.create(0, 0, 0, .5)
                    horizontalAlignment: HorizontalAlignment.Fill
                    topPadding: 2
                    rightPadding: 15
                    bottomPadding: 8
                    leftPadding: 15
                    
                    ImageView {
                        id: quickestLabelType
                        horizontalAlignment: HorizontalAlignment.Left
                        verticalAlignment: VerticalAlignment.Center
                    }
                    
                    ImageView {
                        id: quickestLabelZip
                        visible: false
                        imageSource: "asset:///images/mini_keep.png"
                        horizontalAlignment: HorizontalAlignment.Right
                        translationX: 3
                        translationY: 6
                    }
                    
                    Label {
                        horizontalAlignment: HorizontalAlignment.Center                          
                        id: quickestLabel
                        textStyle.color: Color.White
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

                Container {
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 15
                    verticalAlignment: VerticalAlignment.Bottom

                    Button {
                        text: "Quickest"
                        imageSource: "asset:///images/buttons/quickest.png"
                        onTouch: {
                            if (event.touchType == TouchType.Up) {
                                app.browseBookmark(quickestItem.url, text)
                            }
                        }
                    }                    
	            }
            }
        }
    }
}
