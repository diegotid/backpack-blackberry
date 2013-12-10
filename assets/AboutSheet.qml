
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
	                + "\n- See the icons of the web sites you put in the backpack"
	                + "\n- Backup your bookmarks and share them so they can be restored or loaded on a different device"
	                + "\n- Performance improvements"
	                + "\n- Little interface improvements"
            }
            
            Container {
                topPadding: 40
                horizontalAlignment: HorizontalAlignment.Fill
                
                Button {
                    text: "Rate it!"
                    horizontalAlignment: HorizontalAlignment.Center
                    onClicked: app.launchRating()
                }
            }            
	    }
	}
}
