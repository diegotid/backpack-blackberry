
import bb.cascades 1.0
    
Container {
    layout: DockLayout {}
    
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
    
    Container { // Header
        layout: DockLayout {}
        background: Color.Black
        horizontalAlignment: HorizontalAlignment.Fill
        bottomMargin: 15
        
        Container {
            horizontalAlignment: HorizontalAlignment.Right
            topPadding: 10
            rightPadding: 10
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
        topPadding: 78
	    
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
//                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
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
//                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
                textStyle.color: Color.LightGray
                textStyle.fontSize: FontSize.XSmall
                onTextChanged: {
                    if (text.indexOf("http://") == 0) bookmarkURL.setText(text.substr(7));   
                    else if (text.indexOf("/") == text.length - 1) bookmarkURL.setText(text.substring(0, text.length - 1));
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
//                text: "Maecenas sit amet tellus eros, porttitor rhoncus ipsum. Sed rutrum lacus non dolor tincidunt posuere eget ut tortor. Proin ut nisi metus askdfañsl"
                textStyle.color: Color.White
                textStyle.fontSize: FontSize.Small
                preferredHeight: 300
            }
        }
    }
}
