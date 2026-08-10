import QtQuick
import QtQuick.Window

Window {
    id: window
    width: Screen.width
    height: Screen.height
    visible: true
    color: "white"
    title: "HN Reader"

    property string page: "stories"
    property int margin: Math.max(30, width * 0.035)
    property int bodySize: Math.max(25, width * 0.021)
    property int metaSize: Math.max(19, width * 0.016)
    property int titleSize: Math.max(34, width * 0.029)

    function showComments() {
        page = "comments"
        if (backend.comments.length === 0 && !backend.loadingComments)
            backend.loadComments()
    }

    Connections {
        target: backend
        function onStoryOpened() { page = "article" }
    }

    Rectangle {
        id: topBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: Math.max(88, parent.height * 0.055)
        color: "white"
        border.width: 0

        Text {
            anchors.left: parent.left
            anchors.leftMargin: window.margin
            anchors.verticalCenter: parent.verticalCenter
            text: window.page === "stories" ? "HACKER NEWS" :
                  (window.page === "article" ? "ARTICLE" : "COMMENTS")
            color: "black"
            font.pixelSize: window.metaSize
            font.bold: true
            font.letterSpacing: 2
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: window.margin
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            NavButton {
                visible: window.page !== "stories"
                label: "STORIES"
                onTapped: window.page = "stories"
            }
            NavButton {
                visible: window.page === "comments"
                label: "ARTICLE"
                onTapped: window.page = "article"
            }
            NavButton {
                visible: window.page === "article"
                label: "COMMENTS " + backend.selectedStory.comments
                onTapped: window.showComments()
            }
            NavButton {
                visible: window.page === "stories"
                label: backend.loadingStories ? "LOADING" : "REFRESH"
                enabled: !backend.loadingStories
                onTapped: backend.refreshStories()
            }
            NavButton {
                visible: window.page === "stories"
                label: "CLOSE"
                onTapped: backend.quit()
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 2
            color: "black"
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom

        ListView {
            id: storyList
            anchors.fill: parent
            visible: window.page === "stories"
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: backend.stories

            header: Item {
                width: storyList.width
                height: window.margin * 0.7
            }

            delegate: Item {
                required property int index
                required property var modelData
                width: storyList.width
                height: Math.max(145, storyTitle.implicitHeight + storyMeta.implicitHeight + 52)

                Text {
                    id: rank
                    x: window.margin
                    y: 20
                    width: 55
                    text: (index + 1) + "."
                    color: "#555555"
                    font.pixelSize: window.bodySize
                }
                Text {
                    id: storyTitle
                    anchors.left: rank.right
                    anchors.right: parent.right
                    anchors.rightMargin: window.margin
                    y: 18
                    text: modelData.title
                    color: "black"
                    font.pixelSize: window.bodySize
                    font.family: "sans-serif"
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
                Text {
                    id: storyMeta
                    anchors.left: storyTitle.left
                    anchors.right: storyTitle.right
                    anchors.top: storyTitle.bottom
                    anchors.topMargin: 9
                    text: modelData.score + " points  |  " + modelData.comments + " comments  |  " +
                          modelData.age + "  |  " + modelData.host
                    color: "#444444"
                    font.pixelSize: window.metaSize
                    elide: Text.ElideRight
                }
                Rectangle {
                    anchors.left: storyTitle.left
                    anchors.right: parent.right
                    anchors.rightMargin: window.margin
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#777777"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: backend.openStory(index)
                }
            }
        }

        Flickable {
            id: articleView
            anchors.fill: parent
            visible: window.page === "article"
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: articleColumn.height + window.margin * 2

            Column {
                id: articleColumn
                x: window.margin
                y: window.margin
                width: articleView.width - window.margin * 2
                spacing: 18

                Text {
                    width: parent.width
                    text: backend.selectedStory.title || ""
                    color: "black"
                    font.family: "serif"
                    font.pixelSize: window.titleSize
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
                Text {
                    width: parent.width
                    text: (backend.selectedStory.host || "") + "  |  " +
                          (backend.selectedStory.by || "") + "  |  " +
                          (backend.selectedStory.age || "")
                    color: "#444444"
                    font.pixelSize: window.metaSize
                    wrapMode: Text.WordWrap
                }
                Rectangle { width: parent.width; height: 2; color: "black" }
                StatusMessage {
                    width: parent.width
                    visible: backend.loadingArticle ||
                             (backend.errorMessage !== "" && backend.articleHtml === "")
                    message: backend.loadingArticle ? "Preparing a clean reading view..." : backend.errorMessage
                    action: backend.loadingArticle ? "" : "TRY AGAIN"
                    onTapped: backend.retryArticle()
                }
                Text {
                    width: parent.width
                    visible: !backend.loadingArticle && backend.articleHtml !== ""
                    text: backend.articleHtml
                    textFormat: Text.RichText
                    color: "black"
                    font.family: "serif"
                    font.pixelSize: window.bodySize
                    lineHeight: 1.28
                    lineHeightMode: Text.ProportionalHeight
                    wrapMode: Text.WordWrap
                }
            }
        }

        ListView {
            id: commentList
            anchors.fill: parent
            visible: window.page === "comments"
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: backend.comments
            spacing: 0

            header: Item {
                width: commentList.width
                height: commentHeader.height + window.margin
                Column {
                    id: commentHeader
                    x: window.margin
                    y: window.margin
                    width: commentList.width - window.margin * 2
                    spacing: 14
                    Text {
                        width: parent.width
                        text: backend.selectedStory.title || ""
                        color: "black"
                        font.family: "serif"
                        font.pixelSize: window.titleSize
                        font.bold: true
                        wrapMode: Text.WordWrap
                    }
                    StatusMessage {
                        width: parent.width
                        visible: backend.loadingComments || backend.errorMessage !== ""
                        message: backend.loadingComments ?
                                 "Loading thread... " + backend.comments.length + " comments" :
                                 backend.errorMessage
                        action: backend.loadingComments ? "" : "TRY AGAIN"
                        onTapped: backend.loadComments()
                    }
                    Text {
                        width: parent.width
                        visible: !backend.loadingComments && backend.comments.length === 0 &&
                                 backend.errorMessage === ""
                        text: "No comments yet."
                        color: "#444444"
                        font.pixelSize: window.bodySize
                    }
                    Rectangle { width: parent.width; height: 2; color: "black" }
                }
            }

            delegate: Item {
                required property var modelData
                width: commentList.width
                height: commentBody.implicitHeight + commentMeta.implicitHeight + 58
                property real inset: window.margin + Math.min(modelData.depth, 5) * Math.max(25, window.width * 0.026)

                Rectangle {
                    visible: modelData.depth > 0
                    x: parent.inset - 15
                    y: 18
                    width: 2
                    height: parent.height - 28
                    color: modelData.depth > 2 ? "#999999" : "#555555"
                }
                Text {
                    id: commentMeta
                    x: parent.inset
                    y: 18
                    width: parent.width - x - window.margin
                    text: modelData.by + "  |  " + modelData.age
                    color: "#333333"
                    font.pixelSize: window.metaSize
                    font.bold: true
                }
                Text {
                    id: commentBody
                    x: parent.inset
                    anchors.top: commentMeta.bottom
                    anchors.topMargin: 9
                    width: parent.width - x - window.margin
                    text: modelData.text
                    textFormat: Text.RichText
                    color: "black"
                    font.pixelSize: window.bodySize
                    lineHeight: 1.18
                    lineHeightMode: Text.ProportionalHeight
                    wrapMode: Text.WordWrap
                }
                Rectangle {
                    x: parent.inset
                    anchors.right: parent.right
                    anchors.rightMargin: window.margin
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#888888"
                }
            }
        }

        StatusMessage {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: window.margin
            anchors.verticalCenter: parent.verticalCenter
            visible: window.page === "stories" &&
                     (backend.loadingStories || backend.errorMessage !== "")
            message: backend.loadingStories ? "Loading today's top stories..." : backend.errorMessage
            action: backend.loadingStories ? "" : "TRY AGAIN"
            onTapped: backend.refreshStories()
        }
    }

    component NavButton: Rectangle {
        id: button
        property string label
        signal tapped
        width: buttonText.implicitWidth + 34
        height: Math.max(52, topBar.height - 24)
        color: "white"
        border.width: 1
        border.color: enabled ? "black" : "#888888"
        Text {
            id: buttonText
            anchors.centerIn: parent
            text: button.label
            color: button.enabled ? "black" : "#777777"
            font.pixelSize: window.metaSize
            font.bold: true
        }
        MouseArea {
            anchors.fill: parent
            enabled: button.enabled
            onClicked: button.tapped()
        }
    }

    component StatusMessage: Column {
        id: status
        property string message
        property string action
        signal tapped
        spacing: 20
        Text {
            width: parent.width
            text: status.message
            color: "#333333"
            font.family: "serif"
            font.pixelSize: window.bodySize
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: status.action !== ""
            width: actionText.implicitWidth + 42
            height: 58
            color: "white"
            border.width: 2
            border.color: "black"
            Text {
                id: actionText
                anchors.centerIn: parent
                text: status.action
                color: "black"
                font.pixelSize: window.metaSize
                font.bold: true
            }
            MouseArea { anchors.fill: parent; onClicked: status.tapped() }
        }
    }
}
