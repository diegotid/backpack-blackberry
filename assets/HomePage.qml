
import bb.cascades 1.0

	Page {
	    titleBar: TitleBar {
	        title: "My Backpack"
//	        scrollBehavior: TitleBarScrollBehavior.Sticky // Comment for 10.0
	    }
        
        attachedObjects: [
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
	            id: buttons
	            objectName: "buttons"
                horizontalAlignment: HorizontalAlignment.Center
                verticalAlignment: VerticalAlignment.Center
                             
                Container { // 1st row
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }

                Container {
                    layout: DockLayout {}
                    
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
                }

                Container {
                    layout: DockLayout {}
   
                    ImageView {
                        imageSource: "asset:///images/buttons/lounge.png"
                        onTouch: {
                            if (event.touchType == TouchType.Down) {
                                imageSource = "asset:///images/buttons/lounge-press.png"
                            } else if (event.touchType == TouchType.Up) {
                                imageSource = "asset:///images/buttons/lounge.png"
                                app.loungeBookmark()
                            } else if (event.touchType == TouchType.Cancel) {
                                imageSource = "asset:///images/buttons/lounge.png"
                            }
                        }
                    }
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                }
                        horizontalAlignment: HorizontalAlignment.Right
                
                        Container {
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                            background: toast.imagePaint
                            leftPadding: 28
                            rightPadding: 25
                            
                            Container {
	                            topPadding: 8
	                            bottomPadding: 20
	
	                            Label {
	                                id: loungeLabel
	                                objectName: "loungeLabel"
	                                text: app.getLoungeSize()
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
                            
                            ImageView {
                                id: loungeLabelZip
                                objectName: "loungeLabelZip"
                                visible: false
                                imageSource: "asset:///images/zipMini.png"
                                translationY: 17
                            }
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
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                                background: toast.imagePaint
                            leftPadding: 28
                            rightPadding: 25
                            
                            Container {
                                topPadding: 8
                                bottomPadding: 20
                                
                                Label {
		                            id: oldestLabel
		                            objectName: "oldestLabel"
	                                text: app.getOldestDate().toString("yyyy-MM-dd")
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
                            
                            ImageView {
                                id: oldestLabelZip
                                objectName: "oldestLabelZip"
                                visible: false
                                imageSource: "asset:///images/zipMini.png"
                                translationY: 17
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
                            layout: StackLayout {
                                orientation: LayoutOrientation.LeftToRight
                            }
                                background: toast.imagePaint
                            leftPadding: 28
                            rightPadding: 25
                            
                            Container {
                                topPadding: 8
                                bottomPadding: 20
                                
		                        Label {
		                            id: quickestLabel
		                            objectName: "quickestLabel"
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
                            
	                        ImageView {
	                            id: quickestLabelZip
                                objectName: "quickestLabelZip"
	                            visible: false
	                            imageSource: "asset:///images/zipMini.png"
	                            translationY: 17
	                    }
                    }
                }
            }
		}
	}
}
}
