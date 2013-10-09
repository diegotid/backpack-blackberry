
import bb.cascades 1.0

Page {
    id: about
    
    signal close();
    
    titleBar: TitleBar {
        title: "About"
        dismissAction: ActionItem {
            title: "Close"
            onTriggered: about.close();   
        }
    }
    
    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
        Container {
            topPadding: 40
            rightPadding: 30
            bottomPadding: 40
            leftPadding: 30
            
            Label {
                multiline: true
                text: "Backpack " + app.getAppVersion()
                textStyle.fontWeight: FontWeight.Bold
                textStyle.fontSize: FontSize.Large
            }

			Container {
                topMargin: 20
                ImageView {
                    imageSource: "asset:///images/icon.png"
                }
            }            
            
            Label {
                multiline: true
                text: "New in this version:"
	                + "\n- Lounge mode, for those relaxed moments with time enough for extended readings"
	                + "\n- Put in funcion, so you can start a search for new content to add"
            }
            
            Container {
                topPadding: 40
                horizontalAlignment: HorizontalAlignment.Fill
                
                Button {
                    text: "Rate the app"
                    horizontalAlignment: HorizontalAlignment.Center
                    onClicked: app.launchRating()
                }
            }            
	    }
	}
}