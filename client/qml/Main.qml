import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 480
    height: 720
    visible: true
    title: "LibSorter"
    color: bgPrimary

    readonly property color bgPrimary: "#1E1E1E"
    readonly property color bgCard: "#2D2D2D"
    readonly property color bgInput: "#383838"
    readonly property color accent: "#6B7C5E"
    readonly property color accentHover: "#8A9A7B"
    readonly property color textPrimary: "#E0E0E0"
    readonly property color textSecondary: "#A0A0A0"
    readonly property int radius: 10

    property string statusText: "\u041e\u0436\u0438\u0434\u0430\u043d\u0438\u0435..."
    property color statusColor: textSecondary

    ListModel { id: booksModel }
    ListModel { id: usersModel }

    property var notesMap: ({})
    property int notesVersion: 0

    function getNotes(idx) { return notesMap[idx] !== undefined ? notesMap[idx] : [] }
    function addNote(idx, note) {
        var notes = getNotes(idx).slice()
        notes.push(note)
        var m = notesMap
        m[idx] = notes
        notesMap = m
        notesVersion++
    }
    function removeNote(idx, noteIdx) {
        var notes = getNotes(idx).slice()
        notes.splice(noteIdx, 1)
        var m = notesMap
        m[idx] = notes
        notesMap = m
        notesVersion++
    }

    property string searchQuery: ""
    function matchesSearch(title, author) {
        if (searchQuery === "") return true
        var q = searchQuery.toLowerCase()
        return title.toLowerCase().indexOf(q) !== -1 || author.toLowerCase().indexOf(q) !== -1
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: connectScreen
    }

    function bookStatusColor(s) {
        if (s === "available") return root.accent
        if (s === "borrowed") return "#C7A050"
        return "#C75050"
    }
    function bookStatusText(s) {
        if (s === "available") return "\u041d\u0430 \u043c\u0435\u0441\u0442\u0435"
        if (s === "borrowed") return "\u0412\u0437\u044f\u0442\u0430"
        return "\u0423\u0442\u0435\u0440\u044f\u043d\u0430"
    }

    Connections {
        target: libClient
        function onAuthSuccess() { root.statusText = "\u041f\u043e\u0434\u043a\u043b\u044e\u0447\u0435\u043d\u043e!"; root.statusColor = root.accent }
        function onAuthError(message) { root.statusText = message; root.statusColor = "#C75050" }
        function onConnectionError(message) { root.statusText = "\u041e\u0448\u0438\u0431\u043a\u0430: " + message; root.statusColor = "#C75050" }
        function onBooksReceived(books) {
            booksModel.clear()
            for (var i = 0; i < books.length; i++) {
                var b = books[i]
                booksModel.append({
                    bookId: b.id || "",
                    title: b.title || "",
                    author: (b.authors && b.authors.length > 0) ? b.authors[0] : "",
                    year: b.year ? String(b.year) : "",
                    genre: b.genre || "",
                    pages: b.pages ? String(b.pages) : "",
                    weight: b.weight ? String(b.weight) : "0",
                    location: b.location_id || "",
                    status: b.status || "available",
                    borrowedBy: b.borrowed_by || ""
                })
            }
            stackView.push(booksScreen)
        }
        function onUsersReceived(users) {
            usersModel.clear()
            for (var i = 0; i < users.length; i++) {
                usersModel.append({ userId: users[i].id, username: users[i].username, role: users[i].role })
            }
        }
        function onBookUpdated(book) { libClient.fetchBooks() }
        function onBookDeleted(id) { libClient.fetchBooks() }
    }

    Component {
        id: connectScreen
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                width: parent.width * 0.85
                spacing: 15
                Label {
                    text: "LibSorter"
                    font.pixelSize: 32
                    font.bold: true
                    color: root.textPrimary
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    text: "\u041d\u0435\u0434\u0430\u0432\u043d\u0438\u0435 \u0431\u0438\u0431\u043b\u0438\u043e\u0442\u0435\u043a\u0438"
                    font.pixelSize: 14
                    color: root.textSecondary
                    visible: libClient.recentLibraries.length > 0
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: libClient.recentLibraries.length > 0
                    Repeater {
                        model: libClient.recentLibraries
                        delegate: Rectangle {
                            width: recentCol.implicitWidth + 20
                            height: 50
                            radius: 8
                            color: root.bgCard
                            border.color: root.accent
                            border.width: 1
                            Column {
                                id: recentCol
                                anchors.centerIn: parent
                                spacing: 2
                                Label {
                                    text: modelData.host + ":" + modelData.port
                                    font.pixelSize: 12
                                    color: root.textPrimary
                                    font.bold: true
                                }
                                Label {
                                    text: modelData.code
                                    font.pixelSize: 10
                                    color: root.textSecondary
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    ipField.text = modelData.host
                                    codeField.text = modelData.code
                                    nameField.text = modelData.username
                                }
                            }
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#3A3A3A"
                    visible: libClient.recentLibraries.length > 0
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: connectCol.implicitHeight + 30
                    color: root.bgCard
                    radius: root.radius
                    ColumnLayout {
                        id: connectCol
                        anchors.centerIn: parent
                        width: parent.width - 30
                        spacing: 12
                        Label {
                            text: "IP \u0430\u0434\u0440\u0435\u0441 \u0441\u0435\u0440\u0432\u0435\u0440\u0430"
                            font.pixelSize: 12
                            color: root.textSecondary
                        }
                        TextField {
                            id: ipField
                            Layout.fillWidth: true
                            text: "localhost"
                            color: root.textPrimary
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label {
                            text: "\u041a\u043e\u0434 \u0431\u0438\u0431\u043b\u0438\u043e\u0442\u0435\u043a\u0438"
                            font.pixelSize: 12
                            color: root.textSecondary
                        }
                        TextField {
                            id: codeField
                            Layout.fillWidth: true
                            maximumLength: 8
                            color: root.textPrimary
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label {
                            text: "\u0418\u043c\u044f \u043f\u043e\u043b\u044c\u0437\u043e\u0432\u0430\u0442\u0435\u043b\u044f"
                            font.pixelSize: 12
                            color: root.textSecondary
                        }
                        TextField {
                            id: nameField
                            Layout.fillWidth: true
                            color: root.textPrimary
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label {
                            text: "\u041f\u0430\u0440\u043e\u043b\u044c"
                            font.pixelSize: 12
                            color: root.textSecondary
                        }
                        TextField {
                            id: passField
                            Layout.fillWidth: true
                            echoMode: TextInput.Password
                            color: root.textPrimary
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Button {
                            Layout.fillWidth: true
                            text: "\u041f\u043e\u0434\u043a\u043b\u044e\u0447\u0438\u0442\u044c\u0441\u044f"
                            font.pixelSize: 16
                            font.bold: true
                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: "#FFFFFF"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.hovered ? root.accentHover : root.accent
                                radius: root.radius
                            }
                            onClicked: {
                                if (codeField.text.trim() === "") { createLibDialog.open(); return }
                                if (codeField.text.length !== 8) { root.statusText = "\u041a\u043e\u0434 \u0434\u043e\u043b\u0436\u0435\u043d \u0431\u044b\u0442\u044c 8 \u0441\u0438\u043c\u0432\u043e\u043b\u043e\u0432"; root.statusColor = "#C75050"; return }
                                root.statusText = "\u041f\u043e\u0434\u043a\u043b\u044e\u0447\u0435\u043d\u0438\u0435..."
                                root.statusColor = root.textSecondary
                                libClient.connectToServer(ipField.text, 9443, codeField.text, nameField.text, passField.text)
                            }
                        }
                    }
                }
                Label {
                    text: root.statusText
                    font.pixelSize: 13
                    color: root.statusColor
                    Layout.alignment: Qt.AlignHCenter
                }
            }
            Dialog {
                id: createLibDialog
                anchors.centerIn: parent
                title: "\u0421\u043e\u0437\u0434\u0430\u043d\u0438\u0435 \u0431\u0438\u0431\u043b\u0438\u043e\u0442\u0435\u043a\u0438"
                modal: true
                standardButtons: Dialog.Ok
                Label {
                    text: "\u0427\u0442\u043e\u0431\u044b \u0441\u043e\u0437\u0434\u0430\u0442\u044c \u043d\u043e\u0432\u0443\u044e \u0431\u0438\u0431\u043b\u0438\u043e\u0442\u0435\u043a\u0443, \u0437\u0430\u043f\u0443\u0441\u0442\u0438\u0442\u0435 \u0441\u0435\u0440\u0432\u0435\u0440 \u0432 \u0442\u0435\u0440\u043c\u0438\u043d\u0430\u043b\u0435 \u043a\u043e\u043c\u0430\u043d\u0434\u043e\u0439 serve."
                    wrapMode: Text.Wrap
                    width: parent.width
                }
            }
        }
    }

    Component {
        id: booksScreen
        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "\u041c\u043e\u044f \u0431\u0438\u0431\u043b\u0438\u043e\u0442\u0435\u043a\u0430"
                        font.pixelSize: 24
                        font.bold: true
                        color: root.textPrimary
                        Layout.fillWidth: true
                    }
                    Label {
                        id: countLabel
                        font.pixelSize: 14
                        color: root.textSecondary
                    }
                }
                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: "\u041f\u043e\u0438\u0441\u043a..."
                    color: root.textPrimary
                    background: Rectangle {
                        color: root.bgInput
                        radius: root.radius
                    }
                    onTextChanged: root.searchQuery = text
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: booksModel
                    spacing: 10
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: root.matchesSearch(title, author) ? (bookCol.implicitHeight + 24) : 0
                        visible: root.matchesSearch(title, author)
                        color: root.bgCard
                        radius: root.radius
                        scale: mouseArea.pressed ? 0.98 : 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
                        Behavior on height { NumberAnimation { duration: 150 } }
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            enabled: parent.visible
                            onClicked: stackView.push(bookDetail, {
                                bookId: bookId,
                                bookTitle: title,
                                bookAuthor: author,
                                bookYear: year,
                                bookLocation: location,
                                bookStatus: status,
                                bookGenre: genre,
                                bookPages: pages,
                                bookWeight: weight,
                                bookIndex: index,
                                bookBorrowedBy: borrowedBy
                            })
                        }
                        ColumnLayout {
                            id: bookCol
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 4
                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: title
                                    font.pixelSize: 16
                                    font.bold: true
                                    color: root.textPrimary
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                Rectangle {
                                    width: 10
                                    height: 10
                                    radius: 5
                                    color: root.bookStatusColor(status)
                                }
                            }
                            Label {
                                text: author + " (" + year + ")"
                                font.pixelSize: 13
                                color: root.textSecondary
                            }
                            Label {
                                text: location
                                font.pixelSize: 12
                                color: root.textSecondary
                            }
                        }
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: "+ \u0414\u043e\u0431\u0430\u0432\u0438\u0442\u044c \u043a\u043d\u0438\u0433\u0443"
                    font.pixelSize: 16
                    font.bold: true
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? root.accentHover : root.accent
                        radius: root.radius
                    }
                    onClicked: stackView.push(editScreen, {
                        editId: "",
                        editTitle: "",
                        editAuthor: "",
                        editYear: "",
                        editLocation: "",
                        editGenre: "",
                        editPages: "",
                        editWeight: "",
                        editIndex: -1
                    })
                }
            }
            Timer {
                interval: 100
                running: true
                repeat: true
                onTriggered: {
                    var total = booksModel.count
                    var found = 0
                    for (var i = 0; i < total; i++) {
                        if (root.matchesSearch(booksModel.get(i).title, booksModel.get(i).author))
                            found++
                    }
                    countLabel.text = found + " \u0438\u0437 " + total + " \u043a\u043d\u0438\u0433"
                }
            }
        }
    }

    Component {
        id: bookDetail
        Item {
            property string bookId: ""
            property string bookTitle: ""
            property string bookAuthor: ""
            property string bookYear: ""
            property string bookLocation: ""
            property string bookStatus: ""
            property string bookGenre: ""
            property string bookPages: ""
            property string bookWeight: ""
            property string bookBorrowedBy: ""
            property int bookIndex: -1
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16
                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: "< \u041d\u0430\u0437\u0430\u0434"
                        contentItem: Text {
                            text: parent.text
                            color: root.accent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle { color: "transparent" }
                        onClicked: stackView.pop()
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        width: statusRow.implicitWidth + 20
                        height: 28
                        radius: 14
                        color: root.bookStatusColor(bookStatus)
                        MouseArea {
                            anchors.fill: parent
                            onClicked: statusMenu.open()
                        }
                        Row {
                            id: statusRow
                            anchors.centerIn: parent
                            spacing: 6
                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                color: "#FFFFFF"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Label {
                                text: root.bookStatusText(bookStatus)
                                color: "#FFFFFF"
                                font.pixelSize: 12
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
                Label {
                    text: bookTitle
                    font.pixelSize: 22
                    font.bold: true
                    color: root.textPrimary
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Label {
                    text: bookAuthor
                    font.pixelSize: 16
                    color: root.textSecondary
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#3A3A3A"
                }
                GridLayout {
                    columns: 2
                    columnSpacing: 16
                    rowSpacing: 12
                    Layout.fillWidth: true
                    Label { text: "\u0413\u043e\u0434"; font.pixelSize: 12; color: root.textSecondary }
                    Label { text: bookYear; font.pixelSize: 14; color: root.textPrimary }
                    Label { text: "\u0416\u0430\u043d\u0440"; font.pixelSize: 12; color: root.textSecondary }
                    Label { text: bookGenre || "-"; font.pixelSize: 14; color: root.textPrimary }
                    Label { text: "\u0421\u0442\u0440\u0430\u043d\u0438\u0446"; font.pixelSize: 12; color: root.textSecondary }
                    Label { text: bookPages || "-"; font.pixelSize: 14; color: root.textPrimary }
                    Label { text: "\u0412\u0435\u0441 (\u0433)"; font.pixelSize: 12; color: root.textSecondary }
                    Label { text: bookWeight || "-"; font.pixelSize: 14; color: root.textPrimary }
                    Label { text: "\u0420\u0430\u0441\u043f\u043e\u043b\u043e\u0436\u0435\u043d\u0438\u0435"; font.pixelSize: 12; color: root.textSecondary }
                    Label { text: bookLocation; font.pixelSize: 14; color: root.accent; font.bold: true }
                    Label { text: "\u0412\u0437\u044f\u043b"; font.pixelSize: 12; color: root.textSecondary; visible: bookStatus === "borrowed" }
                    Label { text: bookBorrowedBy || "-"; font.pixelSize: 14; color: root.textPrimary; visible: bookStatus === "borrowed" }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#3A3A3A"
                }
                Label {
                    text: "\u0417\u0430\u043c\u0435\u0442\u043a\u0438"
                    font.pixelSize: 12
                    color: root.textSecondary
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    Repeater {
                        model: root.getNotes(bookIndex)
                        delegate: Rectangle {
                            height: 32
                            width: noteRow.implicitWidth + 16
                            radius: 16
                            color: root.bgInput
                            Row {
                                id: noteRow
                                anchors.centerIn: parent
                                spacing: 6
                                Label {
                                    text: modelData
                                    font.pixelSize: 12
                                    color: root.textPrimary
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Rectangle {
                                    width: 18
                                    height: 18
                                    radius: 9
                                    color: "#555555"
                                    anchors.verticalCenter: parent.verticalCenter
                                    Label {
                                        anchors.centerIn: parent
                                        text: "\u2212"
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: "#FFFFFF"
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.removeNote(bookIndex, index)
                                    }
                                }
                            }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                Row {
                    Layout.fillWidth: true
                    spacing: 10
                    Button {
                        width: (parent.width - 10) / 2
                        text: "\u0423\u0434\u0430\u043b\u0438\u0442\u044c"
                        font.pixelSize: 14
                        contentItem: Text {
                            text: parent.text
                            color: "#C75050"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? "#3A2020" : root.bgInput
                            radius: root.radius
                        }
                        onClicked: {
                            libClient.deleteBook(bookId)
                            stackView.pop()
                        }
                    }
                    Button {
                        width: (parent.width - 10) / 2
                        text: "\u0420\u0435\u0434\u0430\u043a\u0442\u0438\u0440\u043e\u0432\u0430\u0442\u044c"
                        font.pixelSize: 14
                        font.bold: true
                        contentItem: Text {
                            text: parent.text
                            color: "#FFFFFF"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: parent.hovered ? root.accentHover : root.accent
                            radius: root.radius
                        }
                        onClicked: stackView.push(editScreen, {
                            editId: bookId,
                            editTitle: bookTitle,
                            editAuthor: bookAuthor,
                            editYear: bookYear,
                            editLocation: bookLocation,
                            editGenre: bookGenre,
                            editPages: bookPages,
                            editWeight: bookWeight,
                            editIndex: bookIndex
                        })
                    }
                }
            }
            Menu {
                id: statusMenu
                MenuItem {
                    text: "\u0412\u044b\u0434\u0430\u0442\u044c \u043a\u043d\u0438\u0433\u0443"
                    enabled: bookStatus === "available"
                    onTriggered: borrowDialog.open()
                }
                MenuItem {
                    text: "\u0412\u0435\u0440\u043d\u0443\u0442\u044c \u043a\u043d\u0438\u0433\u0443"
                    enabled: bookStatus === "borrowed"
                    onTriggered: {
                        libClient.returnBook(bookId)
                        stackView.pop()
                    }
                }
                MenuItem {
                    text: "\u041f\u043e\u043c\u0435\u0442\u0438\u0442\u044c \u043a\u0430\u043a \u0443\u0442\u0435\u0440\u044f\u043d\u043d\u0443\u044e"
                    enabled: bookStatus !== "lost"
                    onTriggered: lostDialog.open()
                }
            }
            Dialog {
                id: borrowDialog
                anchors.centerIn: parent
                title: "\u041a\u043e\u043c\u0443 \u0432\u044b\u0434\u0430\u0442\u044c?"
                modal: true
                standardButtons: Dialog.Cancel
                width: parent.width * 0.8
                ListView {
                    width: parent.width
                    height: 200
                    model: usersModel
                    clip: true
                    delegate: ItemDelegate {
                        width: parent.width
                        text: username
                        onClicked: {
                            libClient.borrowBook(bookId, userId)
                            borrowDialog.close()
                            stackView.pop()
                        }
                    }
                }
            }
            Dialog {
                id: lostDialog
                anchors.centerIn: parent
                title: "\u041a\u043d\u0438\u0433\u0430 \u0443\u0442\u0435\u0440\u044f\u043d\u0430"
                modal: true
                standardButtons: Dialog.Ok | Dialog.Cancel
                width: parent.width * 0.8
                Column {
                    spacing: 10
                    width: parent.width
                    Label {
                        text: "\u0412\u044b \u0443\u0432\u0435\u0440\u0435\u043d\u044b?"
                        wrapMode: Text.Wrap
                        width: parent.width
                    }
                    TextField {
                        id: lostNote
                        width: parent.width
                        placeholderText: "\u041a\u043e\u043c\u043c\u0435\u043d\u0442\u0430\u0440\u0438\u0439"
                        background: Rectangle {
                            color: root.bgInput
                            radius: 4
                        }
                    }
                }
                onAccepted: {
                    if (lostNote.text.trim() !== "")
                        root.addNote(bookIndex, lostNote.text.trim())
                    stackView.pop()
                }
            }
        }
    }

    Component {
        id: editScreen
        Item {
            property string editId: ""
            property string editTitle: ""
            property string editAuthor: ""
            property string editYear: ""
            property string editLocation: ""
            property string editGenre: ""
            property string editPages: ""
            property string editWeight: ""
            property int editIndex: -1
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14
                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: "< \u041d\u0430\u0437\u0430\u0434"
                        contentItem: Text {
                            text: parent.text
                            color: root.accent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle { color: "transparent" }
                        onClicked: stackView.pop()
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: editId !== "" ? "\u0420\u0435\u0434\u0430\u043a\u0442\u0438\u0440\u043e\u0432\u0430\u043d\u0438\u0435" : "\u041d\u043e\u0432\u0430\u044f \u043a\u043d\u0438\u0433\u0430"
                        font.pixelSize: 16
                        font.bold: true
                        color: root.textPrimary
                    }
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    ColumnLayout {
                        width: parent.width
                        spacing: 12
                        Label { text: "\u041d\u0430\u0437\u0432\u0430\u043d\u0438\u0435 *"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fTitle
                            Layout.fillWidth: true
                            text: editTitle
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label { text: "\u0410\u0432\u0442\u043e\u0440 *"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fAuthor
                            Layout.fillWidth: true
                            text: editAuthor
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label { text: "\u0413\u043e\u0434 *"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fYear
                            Layout.fillWidth: true
                            text: editYear
                            inputMethodHints: Qt.ImhDigitsOnly
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label { text: "\u0416\u0430\u043d\u0440"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fGenre
                            Layout.fillWidth: true
                            text: editGenre
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label { text: "\u0421\u0442\u0440\u0430\u043d\u0438\u0446"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fPages
                            Layout.fillWidth: true
                            text: editPages
                            inputMethodHints: Qt.ImhDigitsOnly
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label { text: "\u0412\u0435\u0441 (\u0433)"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fWeight
                            Layout.fillWidth: true
                            text: editWeight
                            inputMethodHints: Qt.ImhDigitsOnly
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                        Label { text: "\u0420\u0430\u0441\u043f\u043e\u043b\u043e\u0436\u0435\u043d\u0438\u0435"; font.pixelSize: 12; color: root.textSecondary }
                        TextField {
                            id: fLocation
                            Layout.fillWidth: true
                            text: editLocation
                            background: Rectangle {
                                color: root.bgInput
                                radius: root.radius
                            }
                        }
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: editId !== "" ? "\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c" : "\u0414\u043e\u0431\u0430\u0432\u0438\u0442\u044c"
                    font.pixelSize: 16
                    font.bold: true
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.hovered ? root.accentHover : root.accent
                        radius: root.radius
                    }
                    onClicked: {
                        if (fTitle.text.trim() === "" || fAuthor.text.trim() === "" || fYear.text.trim() === "")
                            return
                        if (editId !== "") {
                            libClient.updateBook(editId, fTitle.text, fAuthor.text, parseInt(fYear.text), fGenre.text, parseInt(fPages.text) || 0, parseInt(fWeight.text) || 0, fLocation.text)
                        } else {
                            libClient.addBook(fTitle.text, fAuthor.text, parseInt(fYear.text), fGenre.text, parseInt(fPages.text) || 0, parseInt(fWeight.text) || 0, fLocation.text)
                        }
                        stackView.pop()
                        if (editId !== "")
                            stackView.pop()
                    }
                }
            }
        }
    }
}
