
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
            horizontalAlignment: HorizontalAlignment.Fill
            
            Label {
                multiline: true
                text: "Backpack 3.0"
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
                    + "\n- New design of user interface"
                    + "\n- New reading preamble screen"
                    + "\n- Support for 10.3 Passport, Classic and Leap"
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
