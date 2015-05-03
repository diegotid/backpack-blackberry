
import bb.cascades 1.0

Page {
    id: help
    
    signal close();
    
    titleBar: TitleBar {
        title: "How to..."
        dismissAction: ActionItem {
            title: "Close"
            onTriggered: help.close();   
        }
    }
    
    Container {
        topPadding: 15
        
	    SegmentedControl {
	        
	        Option {
	            id: putOption
	            text: "Put stuff in"
	        }
	        Option {
	            id: readOption
	            text: "Read stuff"
	        }
	        Option {
	            id: backupOption
	            text: "Manage stuff"
	        }
	        onSelectedOptionChanged: {
                putSegment.visible = (selectedOption == putOption)
                readSegment.visible = (selectedOption == readOption)
                backupSegment.visible = (selectedOption == backupOption)
            }
	    }
	    
	    Container {
            id: putSegment
//            visible: false
	
            ScrollView {
                scrollViewProperties.scrollMode: ScrollMode.Vertical
                
                Container {
                
                    Header {
                        title: "How to put stuff in your Backpack from other apps"
                        visible: true
                    }
                    
		            Container {
		                layout: StackLayout {
		                    orientation: LayoutOrientation.LeftToRight
		                }
		                ImageView {
		                    minWidth: 270
		                    maxWidth: 270
		                    imageSource: "asset:///images/share-hint.png"
                            scalingMethod: ScalingMethod.AspectFill
                        }
		                Container {
		                    topPadding: 10
		                    rightPadding: 10
		                    bottomPadding: 0
		                    leftPadding: 10
		                    Label {
		                        multiline: true
		                        textStyle.fontSize: FontSize.Small
		                        text: "From wherever you find something you want to keep to read later, share it with:"
		                        bottomMargin: 10
		                    }
		                    ImageView {
		                        imageSource: "asset:///images/share-sample.png"
		                    }
		                }
		            }
		            		            
                    Header {
                        title: "How to add stuff from your Backpack"
                        visible: true
                    }
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        topPadding: 10
                        leftPadding: 10
                        rightPadding: 30
                        Label {
                            multiline: true
                            textStyle.fontSize: FontSize.Small
                            text: "You can also search for new stuff from the 'Put in' tab"
                        }
                        ImageView {
                            imageSource: "asset:///images/menuicons/ic_doctype_add.png"
                            scalingMethod: ScalingMethod.AspectFit
                            minWidth: 81
                        }
                    }
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        topPadding: 10
                        leftPadding: 10
                        rightPadding: 26
                        Label {
                            multiline: true
                            textStyle.fontSize: FontSize.Small
                            text: "Or sync Backpack with your Pocket account to retrieve your stuff"
                        }
                        ImageView {
                            imageSource: "asset:///images/menuicons/pocket.png"
                            scalingMethod: ScalingMethod.AspectFit
                            maxWidth: 80
                            minWidth: 80
                        }
                    }
		        }    
	        }
	    }
		        
        Container {
            id: readSegment
            visible: false

            ScrollView {
                scrollViewProperties.scrollMode: ScrollMode.Vertical
                
                Container {
                        		            
                    Header {
                        title: "Ways to use Backpack"
                        bottomMargin: 10
                    }
                    
		            Container {
		                layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
		                }
		                maxHeight: 170                
		                ImageView {
		                    imageSource: "asset:///images/buttons/shuffle.png"
		                    scalingMethod: ScalingMethod.AspectFit
		                    minWidth: 170
		                    maxWidth: 170
		                }
		                Container {
		                    topPadding: 12
		                    rightPadding: 10
		                    Label {
		                        multiline: true
		                        textStyle.fontSize: FontSize.Small
		                        text: "<html><b>Shuffle:</b><br/>Let your Backpack randomly give you something to read</html>"
		                    }
		                }
		            }    
		            
		            Container {
		                layout: StackLayout {
		                    orientation: LayoutOrientation.LeftToRight
		                }
		                maxHeight: 170       
		                ImageView {
		                    imageSource: "asset:///images/buttons/quickest.png"
		                    scalingMethod: ScalingMethod.AspectFit
		                    minWidth: 170
		                    maxWidth: 170
		                }
		                Container {
                            topPadding: 12
		                    rightPadding: 10
		                    Label {
		                        multiline: true
		                        textStyle.fontSize: FontSize.Small
		                        text: "<html><b>Quickest:</b><br/>The short stories, for when you only have a minute to spare</html>"
		                    }
		                }
		            }    
		            
		            Container {
		                layout: StackLayout {
		                    orientation: LayoutOrientation.LeftToRight
		                }
		                maxHeight: 170                
		                ImageView {
		                    imageSource: "asset:///images/buttons/lounge.png"
		                    scalingMethod: ScalingMethod.AspectFit
		                    minWidth: 170
		                    maxWidth: 170
		                }
		                Container {
		                    topPadding: 12
		                    rightPadding: 10
		                    Label {
		                        multiline: true
		                        textStyle.fontSize: FontSize.Small
		                        text: "<html><b>Lounge:</b><br/>For those relaxed moments with time enough for extended readings</html>"
		                    }
		                }
		            }    
		            
                    Header {
                        title: "Explore the stuff in your Backpack"
                        topMargin: 15
                        bottomMargin: 10
                    }
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        topPadding: 5
                        bottomPadding: 5
                        leftPadding: 18
                        rightPadding: 18
                        
                        Label {
                            multiline: true
                            textStyle.fontSize: FontSize.Small
                            text: "You may always explore what's in your Backpack from the Explore tab"
                        }
                        
                        ImageView {
                            imageSource: "asset:///images/menuicons/ic_search.png"
                            scalingMethod: ScalingMethod.AspectFill
                            minHeight: 90
                            minWidth: 90
                        }
                    }
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        topPadding: 5
                        bottomPadding: 5
                        leftPadding: 18
                        rightPadding: 25
                        
                        Label {
                            multiline: true
                            textStyle.fontSize: FontSize.Small
                            text: "And access it from other devices if connected to your Pocket account"
                            verticalAlignment: VerticalAlignment.Center
                        }
                        
                        ImageView {
                            imageSource: "asset:///images/menuicons/pocket.png"
                            scalingMethod: ScalingMethod.AspectFit
                            maxWidth: 80
                            minWidth: 80
                        }
                    }
		        }
			}
		}

    	Container {
            id: backupSegment
            visible: false

            ScrollView {
                scrollViewProperties.scrollMode: ScrollMode.Vertical
                
                Container {

                    ImageView {
                        imageSource: "asset:///images/menu-hint.png"
                        scalingMethod: ScalingMethod.AspectFit
                        horizontalAlignment: HorizontalAlignment.Fill
                    }
                    
                    Header {
                        title: "Backup, share and restore your stuff"
                        bottomMargin: 10
                    }
                    
                    Container {
                        topPadding: 5
                        bottomPadding: 5
                        leftPadding: 18
                        rightPadding: 18
                        
                        Label {
                            multiline: true
                            textStyle.fontSize: FontSize.Small
                            text: "<html>From the application menu, select Manage to access the following features:"
                            + "\n&nbsp;- <b>New backup</b>: Generate a backup file containing your current Backpack's stuff"
                            + "\n&nbsp;- <b>Restore</b>: Get back the stuff you had when the selected backup file was created"
                            + "\n&nbsp;- <b>Share</b>: Attach a backup file to a new message to share it, so it can be imported on another device"
                            + "\n&nbsp;- <b>Import from file</b>: Take a backup file from other source so it can be restored on your current Backpack"
                            + "</html>"
                        }
                    }
                    
                    Header {
                        title: "Sync with Pocket"
                        topMargin: 15
                        bottomMargin: 10
                    }
                    
                    Container {
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }
                        topPadding: 5
                        bottomPadding: 5
                        leftPadding: 18
                        rightPadding: 18
                        
                        Label {
                            multiline: true
                            textStyle.fontSize: FontSize.Small
                            text: "When connected to Pocket, content in your Backpack is in the cloud, so you won't lose it"
                        }
                        
                        ImageView {
                            imageSource: "asset:///images/menuicons/pocket.png"
                            scalingMethod: ScalingMethod.AspectFit
                            maxWidth: 80
                            minWidth: 80
                        }
                    }
                }
            }        
        }
    }
}
