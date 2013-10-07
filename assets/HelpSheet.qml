
import bb.cascades 1.0

Page {
    id: help
    
    signal close();
    
    titleBar: TitleBar {
        title: "Help"
        dismissAction: ActionItem {
            title: "Close"
            onTriggered: help.close();   
        }
    }
    
    ScrollView {
        scrollViewProperties.scrollMode: ScrollMode.Vertical
        
        Container {
            
            Header {
                title: "How to put stuff in your Backpack"
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
                    }
                    ImageView {
                        imageSource: "asset:///images/share-sample.png"
                    }
                }
            }    
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                ImageView {
                    minWidth: 270
                    maxWidth: 270
                    imageSource: "asset:///images/putin-hint.png"
                    scalingMethod: ScalingMethod.AspectFill
                }
                Container {
                    topPadding: 10
                    rightPadding: 10
                    leftPadding: 10
                    Label {
                        multiline: true
                        textStyle.fontSize: FontSize.Small
                        text: "You can also search for new stuff from the 'Put in' tab"
                    }
                    ImageView {
                        imageSource: "asset:///images/menuicons/ic_doctype_add.png"
                        translationX: -10
                        translationY: -10
                    }
                }
            }    
            
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
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                maxHeight: 170       
                ImageView {
                    imageSource: "asset:///images/buttons/oldest.png"
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
                        text: "<html><b>Oldest:</b><br/>Read what's been in your Backpack the longest time</html>"
                    }
                }
            }    
        }
	}
}
