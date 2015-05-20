
import bb.cascades 1.4
    
Container {
    layout: DockLayout {}
    background: Color.Black
    
//	Q10
//    maxWidth: 310
//    maxHeight: 211
//	Z10
//    maxWidth: 334
//    maxHeight: 396
//	Z30
//    maxWidth: 319
//    maxHeight: 437

    ImageView {
        id: bookmarkPic
        objectName: "bookmarkPic"
        imageSource: "asset:///images/backpack.png"
        onImageSourceChanged: {
            if (imageSource.toString().length <= 1) {
                bookmarkPic.imageSource = "asset:///images/backpack.png"
            }            
        }
        scalingMethod: ScalingMethod.AspectFill
        verticalAlignment: VerticalAlignment.Fill
        horizontalAlignment: HorizontalAlignment.Fill
    }
    
    ImageView {
        imageSource: "asset:///images/shadow.png"
        scalingMethod: ScalingMethod.AspectFill
        verticalAlignment: VerticalAlignment.Fill
        horizontalAlignment: HorizontalAlignment.Fill
        opacity: 0.75
    }

    Container {
        layout: StackLayout {}
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Fill

        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            topPadding: 10
            rightPadding: 10
            bottomPadding: 10
            leftPadding: 10
            horizontalAlignment: HorizontalAlignment.Left
            verticalAlignment: VerticalAlignment.Top
            
            ImageView {
                id: bookmarkIcon
                objectName: "bookmarkIcon"
//                imageSource: "asset:///images/favicon.png"
                minHeight: 24
                minWidth: 24
                verticalAlignment: VerticalAlignment.Center
                translationY: 2
                visible: imageSource && imageSource.toString().length > 0
            }
            
            Label {
                id: bookmarkURL
                objectName: "bookmarkURL"
//                text: "www.bbornot2b.com"
                textStyle.color: Color.White
                textStyle.fontSize: FontSize.XSmall
                leftMargin: 10
                onTextChanged: {
                    var domain = text.substring(text.indexOf("://") + (text.indexOf("://") < 0 ? 1 : 3));
                    if (domain.indexOf('/') < domain.indexOf('.')) {
                        bookmarkURL.setText(domain)
                    } else {
                        bookmarkURL.setText(domain.substring(0, domain.indexOf('/')))
                    }
                }
            }
        }

        Container {
            rightPadding: 10
            leftPadding: 10
            bottomPadding: ui.sdu(2)
            layout: DockLayout {}
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }

            Label {
                id: bookmarkTitle
                objectName: "bookmarkTitle"
                multiline: true
                verticalAlignment: VerticalAlignment.Bottom
//                text: "Probando por aquí"
//                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
                textStyle.color: Color.White
                textStyle.fontWeight: FontWeight.Bold
            }
	    }
    }
}
