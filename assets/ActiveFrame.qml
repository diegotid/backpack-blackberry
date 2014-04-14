
import bb.cascades 1.0
    
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

	attachedObjects: LayoutUpdateHandler {
        id: frameHandler
    }
       
    ImageView {
        id: bookmarkPic
        objectName: "bookmarkPic"
        imageSource: "asset:///images/backpack.png"
        onImageSourceChanged: {
            if (imageSource.toString().length == 0) {
                bookmarkPic.imageSource = "asset:///images/backpack.png"
            }            
        }
        scalingMethod: ScalingMethod.AspectFill
        verticalAlignment: VerticalAlignment.Fill
        horizontalAlignment: HorizontalAlignment.Fill
    }
    
    ImageView {
        imageSource: "asset:///images/frame-shadow.png"
        scalingMethod: ScalingMethod.AspectFill
        verticalAlignment: VerticalAlignment.Fill
        horizontalAlignment: HorizontalAlignment.Fill
        opacity: 0.75
    }
    
    Container { // Header
        layout: DockLayout {}
        horizontalAlignment: HorizontalAlignment.Fill
        background: Color.create(0, 0, 0, 0.6)
        
        Container {
            topPadding: 8
            rightPadding: 10
            horizontalAlignment: HorizontalAlignment.Right
            ImageView {
                imageSource: "asset:///images/small_icon.png"
            }        
        }

        Container {
            topPadding: 6
            bottomPadding: 10
            rightPadding: 10
            leftPadding: 10
            Label {
                objectName: "oldest"
//                text: "Oldest: 1 week ago"
                textStyle.color: Color.White
                textStyle.fontSize: FontSize.XXSmall
                bottomMargin: 0
            } 
            Label {
                objectName: "quickest"
                topMargin: 0
//                text: "Quickest: 3 min"
                textStyle.color: Color.White
                textStyle.fontSize: FontSize.XXSmall
            }             
        }
    }

    Container {
        layout: DockLayout {}
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Fill
        topPadding: 70

        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            topPadding: 10
            rightPadding: 10
            bottomPadding: 10
            leftPadding: 10
            horizontalAlignment: HorizontalAlignment.Left
            verticalAlignment: VerticalAlignment.Bottom
            
            attachedObjects: LayoutUpdateHandler {
                id: footHandler
            }

            ImageView {
                id: bookmarkIcon
                objectName: "bookmarkIcon"
                imageSource: "asset:///images/favicon.png"
                minHeight: 24
                minWidth: 24
                verticalAlignment: VerticalAlignment.Center
                translationY: 2
            }
            
            Label {
                id: bookmarkURL
                objectName: "bookmarkURL"
//                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
                textStyle.color: Color.White
                textStyle.fontSize: FontSize.XSmall
                leftMargin: 10
                onTextChanged: {
                    if (text.toString().length > 0) {
                        if (text.indexOf("http://") == 0)
                            bookmarkURL.setText(text.substr(7));   
                        else if (text.indexOf("/") == text.toString().length - 1)
                            bookmarkURL.setText(text.substring(0, text.toString().length - 1));
                    }
                }
            }
        }

        Container {
            rightPadding: 10
            leftPadding: 10
            verticalAlignment: VerticalAlignment.Bottom
            bottomPadding: footHandler.layoutFrame.height

            Container {
	            id: titleCase
	            visible: bookmarkTitle.text.toString().length > 0
	            
	            Label {
	                id: bookmarkTitle
	                objectName: "bookmarkTitle"
	                multiline: true
//	                text: "Probando por aquí"
//	                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
	                textStyle.color: Color.White
	                textStyle.fontWeight: FontWeight.Bold
	            }
	        }        
	        	        
	        Container {
	            id: memoCase
	            visible: memo.text.toString().length > 0
	            
	            Label {
	                id: memo
	                objectName: "memo"
	                multiline: true
//	                text: "Y un comentario por acá"
//	                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
	                textStyle.color: Color.White
	                textStyle.fontSize: FontSize.Small
	                preferredHeight: 300
	            }
	        }
	    }
    }
}
