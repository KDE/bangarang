import QtQuick 1.0

ListView {
    id:container
    width: 500
    height: 400

    SystemPalette { id: activePalette; colorGroup: SystemPalette.Active }

    Component {
        id: itemDelegate
        Rectangle {
            id: wrapper
            width: container.width
            height: container.height
            gradient: ListView.isCurrentItem ? selected : notSelected

            Image {
                id: artwork
                source: artworkId
                sourceSize.width: 128
                sourceSize.height: 128
            }

            Image {
                id: reflection
                anchors.top: artwork.bottom
                source: artworkId + "/reflection"
                sourceSize.width: 128
                sourceSize.height: 128
            }

            Text {
                id: titleText
                anchors.left: artwork.right
                anchors.bottom: subTitleText.top
                text: title
                font.bold: true
                font.pointSize: 16
                color: wrapper.ListView.isCurrentItem ? "white" : "black"
            }
            Text {
                id: subTitleText
                anchors.left: artwork.right
                anchors.bottom: ratingView.top
                text: subTitle
                color: wrapper.ListView.isCurrentItem ? "white" : "black"
            }
            Text {
                id:ratingView
                anchors.left: artwork.right
                anchors.bottom: artwork.bottom
                text: rating
                color: wrapper.ListView.isCurrentItem ? "white" : "black"
            }
            Gradient {
                id:selected
                GradientStop { position: 0.0; color: "black" }
                GradientStop { position: 0.5; color: activePalette.highlight }
                GradientStop { position: 1.0; color: "black" }
            }
            Gradient {
                id:notSelected
                GradientStop { position: 0.0; color: "#DDDDDD" }
                GradientStop { position: 1.0; color: "#DDDDDD" }
            }
        }
    }


    model: nowPlayingModel
    delegate: itemDelegate
}
