
import bb.cascades 1.0

Page {
    id: settings
    
    signal close();
    
    property bool prevKeptShuffle: app.getIgnoreKeptShuffle()
    property bool prevKeptOldest: app.getIgnoreKeptOldest()
    property bool prevKeptQuickest: app.getIgnoreKeptQuickest()

    titleBar: TitleBar {
        title: "Settings"
        dismissAction: ActionItem {
            title: "Cancel"
            onTriggered: {
                ignoreKeptShuffle.checked = prevKeptShuffle;
                ignoreKeptOldest.checked = prevKeptOldest;
                ignoreKeptQuickest.checked = prevKeptQuickest;
                settings.close();   
            }
        }
        acceptAction: ActionItem {
            title: "Confirm"
            onTriggered: {
                app.setBackgroundColour(originalSlider.value, redSlider.value, greenSlider.value, blueSlider.value)
                homePage.reloadBackgrounds();
                app.setIgnoreKeptShuffle(ignoreKeptShuffle.checked);
                app.setIgnoreKeptOldest(ignoreKeptOldest.checked);
                app.setIgnoreKeptQuickest(ignoreKeptQuickest.checked);
                prevKeptShuffle = ignoreKeptShuffle.checked;
                prevKeptOldest = ignoreKeptOldest.checked;
                prevKeptQuickest = ignoreKeptQuickest.checked;
                settings.close();
            }
        }
    }
    
    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
        Container {
            
            Header {
                title: "Ignore kept bookmarks for..."
                subtitle: "(if no kept exist)"
            }
            
            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                leftPadding: 20.0
                rightPadding: 20.0
                
                Label {
                    text: "Shuffle"
                    verticalAlignment: VerticalAlignment.Center
                }
                
                ToggleButton {
                    id: ignoreKeptShuffle
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getIgnoreKeptShuffle();
                }
            }
            
            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                leftPadding: 20.0
                rightPadding: 20.0
                
                Label {
                    text: "Oldest"
                    verticalAlignment: VerticalAlignment.Center
                }
                
                ToggleButton {
                    id: ignoreKeptOldest
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getIgnoreKeptOldest();
                }
            }
            
            Container {
                layout: DockLayout {}
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                leftPadding: 20.0
                rightPadding: 20.0
                bottomMargin: 10.0
                
                Label {
                    text: "Quickest"
                    verticalAlignment: VerticalAlignment.Center
                }
                
                ToggleButton {
                    id: ignoreKeptQuickest
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Center
                    checked: app.getIgnoreKeptQuickest();
                }
            }    

	        Header {
	            title: "Backpack color"
	        }
	                
	        Container {
	            layout: AbsoluteLayout {}
	            maxHeight: 520
	            
	            ImageView {
	                id: blue
	                imageSource: "asset:///images/background-blue.png"
	            }
	            ImageView {
	                id: red
	                imageSource: "asset:///images/background-red.png"
	            }
	            ImageView {
	                id: green
	                imageSource: "asset:///images/background-green.png"
	            }
	            ImageView {
	                id: original
	                imageSource: "asset:///images/background.png"
	            }
	            Container {
	                id: colourSliders
	                horizontalAlignment: HorizontalAlignment.Fill
	                background: colours.imagePaint
	                minHeight: 520
	
	                attachedObjects: [
	                    ImagePaintDefinition {
	                        id: colours
	                        imageSource: "asset:///images/background-colours.png"
	                    },
	                    ImagePaintDefinition {
	                        id: coloursLefty
	                        imageSource: "asset:///images/background-colours-lefty.png"
	                    }
	                ] 
	                
	                Container {
	                    background: shadow.imagePaint
	                    attachedObjects: [
	                        ImagePaintDefinition {
	                            id: shadow
	                            imageSource: "asset:///images/shadow.png"
	                            repeatPattern: RepeatPattern.X
	                        }
	                    ]
	                    topPadding: 40
	                    rightPadding: 70
	                    bottomPadding: 38
	                    leftPadding: 460
	                    
	                    Slider {
	                        id: originalSlider
	                        value: app.getBackgroundColour("base")
	                        onImmediateValueChanged: {
	                            original.opacity = immediateValue
	                        }
	                        onTouch: {
	                            if (event.touchType == TouchType.Move) {
	                                var rest = blueSlider.value + redSlider.value + greenSlider.value
	                                var gap = 1 - immediateValue - rest
	                                blueSlider.value = blueSlider.value + blueSlider.value * gap / rest
	                                redSlider.value = redSlider.value + redSlider.value * gap / rest
	                                greenSlider.value = greenSlider.value + greenSlider.value * gap / rest
	                            }
	                        }
	                    }
	                }
	                Container {
	                    topPadding: 30
	                    rightPadding: 70
	                    bottomPadding: 38
	                    leftPadding: 460
	                    
	                    Slider {
	                        id: redSlider
	                        value: app.getBackgroundColour("red")
	                        onImmediateValueChanged: {
	                            red.opacity = immediateValue
	                        }
	                        onTouch: {
	                            if (event.touchType == TouchType.Move) {
	                                var rest = blueSlider.value + originalSlider.value + greenSlider.value
	                                var gap = 1 - immediateValue - rest
	                                blueSlider.value = blueSlider.value + blueSlider.value * gap / rest
	                                originalSlider.value = originalSlider.value + originalSlider.value * gap / rest
	                                greenSlider.value = greenSlider.value + greenSlider.value * gap / rest
	                            }
	                        }
	                    }
	                }
	                Container {
	                    topPadding: 30
	                    rightPadding: 70
	                    bottomPadding: 38
	                    leftPadding: 460
	
	                    layout: StackLayout {
	                        orientation: LayoutOrientation.LeftToRight
	                    }
	                    Slider {
	                        id: greenSlider
	                        value: app.getBackgroundColour("green")
	                        onImmediateValueChanged: {
	                            green.opacity = immediateValue
	                        }
	                        onTouch: {
	                            if (event.touchType == TouchType.Move) {
	                                var rest = blueSlider.value + redSlider.value + originalSlider.value
	                                var gap = 1 - immediateValue - rest
	                                blueSlider.value = blueSlider.value + blueSlider.value * gap / rest
	                                redSlider.value = redSlider.value + redSlider.value * gap / rest
	                                originalSlider.value = originalSlider.value + originalSlider.value * gap / rest
	                            }
	                        }
	                    }
	                }
	                Container {
	                    topPadding: 30
	                    rightPadding: 70
	                    bottomPadding: 48
	                    leftPadding: 460
	                    
	                    Slider {
	                        id: blueSlider
	                        value: app.getBackgroundColour("blue")
	                        onImmediateValueChanged: {
	                            blue.opacity = immediateValue
	                        }
	                        onTouch: {
	                            if (event.touchType == TouchType.Move) {
	                                var rest = originalSlider.value + redSlider.value + greenSlider.value
	                                var gap = 1 - immediateValue - rest
	                                originalSlider.value = originalSlider.value + originalSlider.value * gap / rest
	                                redSlider.value = redSlider.value + redSlider.value * gap / rest
	                                greenSlider.value = greenSlider.value + greenSlider.value * gap / rest
	                            }
	                        }
	                    }
	                }
	            }
	
	            Container {
	                id: toggleLefty
	                layout: StackLayout {
	                    orientation: LayoutOrientation.LeftToRight
	                }
	                translationY: 410
	                topPadding: 20
	                rightPadding: 25
	                bottomPadding: 20
	                scaleX: 0.9
	                scaleY: 0.9
	                
	                horizontalAlignment: HorizontalAlignment.Left
	                ToggleButton {
	                    onCheckedChanged: {
	                        originalSlider.translationX = -395 * checked                
	                        redSlider.translationX = -395 * checked               
	                        greenSlider.translationX = -395 * checked                
	                        blueSlider.translationX = -395 * checked
	                        colourSliders.background = checked ? coloursLefty.imagePaint : colours.imagePaint
	                        toggleLefty.translationX = 360 * checked                
	                    }
	                }  
	                Label {
	                    text: "I'm a lefty"
	                    verticalAlignment: VerticalAlignment.Center
	                }
	            }
	        }
	    }
	}
}
