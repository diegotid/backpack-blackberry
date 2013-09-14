
import bb.cascades 1.0

NavigationPane {
    id: homePage
    objectName: "homePage"
    
    onPopTransitionEnded: {
        backButtonsVisible = true
    }
        
    attachedObjects: [
        Sheet {
            id: settingsSheet
            SettingsSheet {
                onClose: settingsSheet.close();
            }
        },
//        Sheet {
//            id: aboutSheet
//            AboutSheet {
//                onClose: aboutSheet.close();
//            }
//        },
        BrowsePage {
            id: browsePage
            objectName: "browsePage"
        }
    ]
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered: settingsSheet.open();
        }
    }
    
    function reloadBackgrounds() {
        for (var i = 0; i < count(); i++) {
            at(i).loadBackground();
        }
        browsePage.loadBackground();
    }
        
	Page {
	    titleBar: TitleBar {
	        title: "My Backpack"
	        scrollBehavior: TitleBarScrollBehavior.Sticky // Comment for 10.0
	    }
        
        attachedObjects: [
            ImagePaintDefinition {
                id: background
                imageSource: "asset:///images/shadow.png"
                repeatPattern: RepeatPattern.X
            },
            ImagePaintDefinition {
                id: toast
                imageSource: "asset:///images/toast.amd"
            }
        ]
	    
        onCreationCompleted: loadBackground()
        
        function loadBackground() {
            backgroundRed.opacity = app.getBackgroundColour("red");
            backgroundGreen.opacity = app.getBackgroundColour("green");
            backgroundBlue.opacity = app.getBackgroundColour("blue");
            backgroundBase.opacity = app.getBackgroundColour("base");
        }

        Container {
	        layout: DockLayout {}
	        
            ImageView {
                id: backgroundRed
                imageSource: "asset:///images/background-red.png"
            }
            ImageView {
                id: backgroundGreen
                imageSource: "asset:///images/background-green.png"
            }
            ImageView {
                id: backgroundBlue
                imageSource: "asset:///images/background-blue.png"
            }
            ImageView {
                id: backgroundBase
                imageSource: "asset:///images/background.png"
            }
	        
            Container {
			    id: hint
			    objectName: "hint"
                topPadding: 30
                bottomPadding: 30
                leftPadding: 30
                rightPadding: 30
                background: background.imagePaint
                horizontalAlignment: HorizontalAlignment.Fill
                visible: false
			    
			    Label {
			        multiline: true	            
			        text: "<html>Nothing in your backpack<br/><br/>To put content in your backpack that you want to read later, just use the share menu option from your browser on any page and select &quot;My Backpack&quot;</html>"
			    }
			    
                ImageView {
                    imageSource: "asset:///images/empty-hint.png"
                    horizontalAlignment: HorizontalAlignment.Center
                    topMargin: 35
			    }
			}
		        
	        Container {
	            id: buttons
	            objectName: "buttons"
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment: VerticalAlignment.Center
//                visible: false
                             
                Container { // 1st row
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }

                    ImageView {
                        imageSource: "asset:///images/buttons/shuffle.png"
                        onTouch: {
                            if (event.touchType == TouchType.Down) {
                                imageSource = "asset:///images/buttons/shuffle-press.png"
                            } else if (event.touchType == TouchType.Up) {
                                imageSource = "asset:///images/buttons/shuffle.png"
                                app.shuffleBookmark()
                            } else if (event.touchType == TouchType.Cancel) {
                                imageSource = "asset:///images/buttons/shuffle.png"
                            }
                        }
                    }
                    ImageView {
                        imageSource: "asset:///images/buttons/browse.png"
                        onTouch: {
                            if (event.touchType == TouchType.Down) {
                                imageSource = "asset:///images/buttons/browse-press.png"
                            } else if (event.touchType == TouchType.Up) {
                                imageSource = "asset:///images/buttons/browse.png"
                                homePage.backButtonsVisible = true
                                homePage.push(browsePage);
                            } else if (event.touchType == TouchType.Cancel) {
                                imageSource = "asset:///images/buttons/browse.png"
                            }
                        }
                    }
                }
                
                Container { // 2nd row
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }

                    Container {
                        layout: DockLayout {}
                        
                        ImageView {
                            imageSource: "asset:///images/buttons/oldest.png"
                            onTouch: {
                                if (event.touchType == TouchType.Down) {
                                    imageSource = "asset:///images/buttons/oldest-press.png"
                                } else if (event.touchType == TouchType.Up) {
                                    imageSource = "asset:///images/buttons/oldest.png"
                                    app.oldestBookmark()
                                } else if (event.touchType == TouchType.Cancel) {
                                    imageSource = "asset:///images/buttons/oldest.png"
                                }
                            }
                        }
                        
                       Container {
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                            horizontalAlignment: HorizontalAlignment.Right

                            Container {
                                background: toast.imagePaint
                                topPadding: 8
                                rightPadding: 25
                                bottomPadding: 20
                                leftPadding: 25
                                
                                Label {
		                            id: oldestLabel
		                            objectName: "oldestLabel"
//		                            text: "Yesterday"
                                    text: app.getOldestDate()
                                    textStyle.fontSize: FontSize.Medium
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
                    }

                    Container {
                        layout: DockLayout {}
                        
                        ImageView {
                            imageSource: "asset:///images/buttons/quickest.png"
                            onTouch: {
                                if (event.touchType == TouchType.Down) {
                                    imageSource = "asset:///images/buttons/quickest-press.png"
                                } else if (event.touchType == TouchType.Up) {
                                    imageSource = "asset:///images/buttons/quickest.png"
                                    app.quickestBookmark()
                                } else if (event.touchType == TouchType.Cancel) {
                                    imageSource = "asset:///images/buttons/quickest.png"
                                }
                            }
                        }
                        
                        Container {
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                            horizontalAlignment: HorizontalAlignment.Right
                            
                            Container {
                                background: toast.imagePaint
                                topPadding: 8
                                rightPadding: 25
                                bottomPadding: 20
                                leftPadding: 25
                                
		                        Label {
		                            id: quickestLabel
		                            objectName: "quickestLabel"
//	                                text: "< 1 min"
		                            text: app.getQuickestSize()
                                    textStyle.fontSize: FontSize.Medium
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
	                    }
                    }
                }
            }
		}
	}
}
