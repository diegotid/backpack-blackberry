
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
       
    ImageView {
        objectName: "backgroundRed"
        imageSource: "asset:///images/frame-red.png"
    }
    ImageView {
        objectName: "backgroundGreen"
        imageSource: "asset:///images/frame-green.png"
    }
    ImageView {
        objectName: "backgroundBlue"
        imageSource: "asset:///images/frame-blue.png"
    }
    ImageView {
        objectName: "backgroundBase"
        imageSource: "asset:///images/frame.png"
    }
    ImageView {
        imageSource: "asset:///images/frame-shadow.png"
    }
    
    Container { // Header
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        horizontalAlignment: HorizontalAlignment.Fill
        background: Color.Black
        
        Container {
            topPadding: 10
            leftPadding: 10
            ImageView {
                imageSource: "asset:///images/small_icon.png"
            }        
        }

        Container {
            topPadding: 10
            bottomPadding: 10
            rightPadding: 10
            leftPadding: 10
            Label {
                objectName: "oldest"
//                text: "Oldest: 1 week ago"
                textStyle.color: Color.LightGray
                textStyle.fontSize: FontSize.XXSmall
                bottomMargin: 0
            } 
            Label {
                objectName: "quickest"
                topMargin: 0
//                text: "Quickest: 3 min"
                textStyle.color: Color.LightGray
                textStyle.fontSize: FontSize.XXSmall
            }             
        }
    }

    Container {
        layout: DockLayout {}
        horizontalAlignment: HorizontalAlignment.Fill
        topPadding: 78
        bottomPadding: 12

        Container {
            topPadding: 10
            rightPadding: 10
            bottomPadding: 10
            leftPadding: 10
            horizontalAlignment: HorizontalAlignment.Right
            ImageView {
                id: bookmarkIcon
                objectName: "bookmarkIcon"
                imageSource: "asset:///images/favicon.png"
                minHeight: 36
                minWidth: 36		    
            }
        }

        Container {
		    
	        Container {
	            id: titleCase
	            topPadding: 0
	            bottomPadding: 5
	            rightPadding: 10
	            leftPadding: 10
	            Label {
	                id: bookmarkTitle
	                objectName: "bookmarkTitle"
	                multiline: true
//	                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
	                textStyle.color: Color.White
	                textStyle.fontWeight: FontWeight.Bold
	            }
	        }        
	        
	        Container {
	            id: urlCase
	            topPadding: 0
	            bottomPadding: 5
	            rightPadding: 10
	            leftPadding: 10
	            Label {
	                id: bookmarkURL
	                objectName: "bookmarkURL"
//	                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
	                textStyle.color: Color.LightGray
	                textStyle.fontSize: FontSize.XSmall
	                onTextChanged: {
	                    if (text.length > 0) {
	                        if (text.indexOf("http://") == 0)
	                        	bookmarkURL.setText(text.substr(7));   
	                        else if (text.indexOf("/") == text.length - 1)
	                        	bookmarkURL.setText(text.substring(0, text.length - 1));
	                    }
	                }
	            }
	        }
	        
	        Container {
	            id: memoCase
	            topPadding: 0
	            bottomPadding: 30
	            rightPadding: 10
	            leftPadding: 10
	            Label {
	                id: memo
	                objectName: "memo"
	                multiline: true
//	                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
	                textStyle.color: Color.White
	                textStyle.fontSize: FontSize.Small
	                preferredHeight: 300
	            }
	        }
	    }
    }
}
