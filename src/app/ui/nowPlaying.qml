import QtQuick 1.0

ListView {
    id:container
    height: 400
    width: 500

    SystemPalette { id: activePalette; colorGroup: SystemPalette.Active }

    property string highlightColor: activePalette.highlight

    Component {
        id: itemDelegate
        Rectangle {
            id: wrapper
            width: container.width
            height: container.height
            gradient: background

            Image {
                id: artwork
                source: artworkId
                anchors.bottom: wrapper.bottom
                anchors.bottomMargin: wrapper.height/2
                anchors.left: wrapper.left
                anchors.leftMargin: 10
                sourceSize.width: 128
                sourceSize.height: 128
            }

            Image {
                id: reflection
                anchors.top: artwork.bottom
                anchors.left: artwork.left
                anchors.topMargin: 1
                source: artworkId + "/reflection"
                sourceSize.width: 128
                sourceSize.height: 128
            }

            Rectangle {
                id: titleRating
                anchors.left: artwork.right
                anchors.bottom: artwork.bottom
                anchors.leftMargin: 10

                Text {
                    id: titleText
                    height:  artwork.height/3
                    anchors.bottom: subTitleText.top
                    anchors.bottomMargin: 5
                    text: title
                    verticalAlignment: Text.AlignBottom
                    font.pixelSize: artwork.height/6
                    color: "white"
                }
                Text {
                    id: subTitleText
                    height:  artwork.height/3
                    anchors.bottom: ratingView.top
                    anchors.bottomMargin: 5
                    text: subTitle
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: artwork.height/9
                    color: "gray"
                }
                Rectangle {
                    id:ratingView
                    anchors.bottom: titleRating.bottom
                    height: 16
                    Row {
                        anchors.bottom: ratingView.bottom
                        Repeater {
                            model: (type == "Audio" || type == "Video") ? 5 : 0
                            Image {
                                id:ratingStarNotRated
                                source: "image://icons/rating/disabled"
                                sourceSize.width: 16
                                sourceSize.height: 16
                            }
                        }
                    }
                    Row {
                        anchors.bottom: ratingView.bottom
                        Repeater {
                            model: rating/2
                            Image {
                                id:ratingStar
                                source: "image://icons/rating"
                                sourceSize.width: 16
                                sourceSize.height: 16
                            }
                        }
                    }
                }

            }

            Rectangle {
                id:ratingViewReflection
                anchors.top: titleRating.bottom
                anchors.left: titleRating.left
                anchors.topMargin: 1
                height: 16
                opacity: 0.2
                Row {
                    anchors.bottom: ratingView.bottom
                    Repeater {
                        model: (type == "Audio" || type == "Video") ? 5 : 0
                        Image {
                            id:ratingStarNotRatedReflection
                            source: "image://icons/rating/disabled"
                            rotation: 180
                            sourceSize.width: 16
                            sourceSize.height: 16
                        }
                    }
                }
                Row {
                    anchors.bottom: ratingView.bottom
                    Repeater {
                        model: rating/2
                        Image {
                            id:ratingStarReflection
                            source: "image://icons/rating"
                            rotation: 180
                            sourceSize.width: 16
                            sourceSize.height: 16
                        }
                    }
                }
            }


            Gradient {
                id:background
                GradientStop { position: 0.0; color: Qt.black }
                GradientStop { position: 0.5; color: "#33" + highlightColor.substring(1) }
                GradientStop { position: 1.0; color: Qt.black }
            }
        }
    }


    model: nowPlayingModel
    delegate: itemDelegate
}
