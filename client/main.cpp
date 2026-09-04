#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QSettings>

class LibSorterClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)
    Q_PROPERTY(QVariantList recentLibraries READ recentLibraries NOTIFY recentLibrariesChanged)

public:
    explicit LibSorterClient(QObject *parent = nullptr) : QObject(parent) {
        loadRecent();
    }

    bool connected() const { return m_connected; }
    QString token() const { return m_token; }
    QVariantList recentLibraries() const { return m_recent; }

    Q_INVOKABLE void connectToServer(const QString &host, int port, const QString &code, const QString &username, const QString &password) {
        if (m_ws) {
            m_ws->close();
            m_ws->deleteLater();
            m_ws = nullptr;
        }

        m_ws = new QWebSocket();
        saveRecent(host, port, code, username);

        QObject::connect(m_ws, &QWebSocket::connected, this, [this, code, username, password]() {
            m_connected = true;
            emit connectedChanged();

            QJsonObject payload;
            payload["code"] = code;
            payload["username"] = username;
            payload["password"] = password;

            QJsonObject msg;
            msg["type"] = "request";
            msg["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
            msg["action"] = "auth.connect";
            msg["payload"] = payload;

            m_ws->sendTextMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
        });

        QObject::connect(m_ws, &QWebSocket::textMessageReceived, this, [this](const QString &message) {
            QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
            QJsonObject obj = doc.object();
            if (obj["type"].toString() != "response") return;

            QString id = obj["id"].toString();
            QJsonObject payload = obj["payload"].toObject();

            if (payload.contains("token")) {
                m_token = payload["token"].toString();
                emit tokenChanged();
                emit authSuccess();
                fetchBooks();
                fetchUsers();
            } else if (payload.contains("code") && !payload.contains("books") && !payload.contains("users")) {
                emit authError(payload["message"].toString());
            } else if (payload.contains("books")) {
                emit booksReceived(payload["books"].toArray());
            } else if (payload.contains("users")) {
                emit usersReceived(payload["users"].toArray());
            } else if (payload.contains("id") && payload.contains("title")) {
                emit bookUpdated(payload);
            } else if (id.startsWith("del-")) {
                emit bookDeleted(id.mid(4));
            }
        });

        QObject::connect(m_ws, &QWebSocket::disconnected, this, [this]() {
            m_connected = false;
            m_token.clear();
            emit connectedChanged();
            emit tokenChanged();
        });

        QObject::connect(m_ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
                this, [this](QAbstractSocket::SocketError) {
            emit connectionError(m_ws->errorString());
        });

        QString url = QString("ws://%1:%2").arg(host).arg(port);
        m_ws->open(QUrl(url));
    }

    Q_INVOKABLE void disconnectFromServer() { if (m_ws) m_ws->close(); }
    Q_INVOKABLE void fetchBooks() { sendRequest("book.list", QJsonObject{}); }
    Q_INVOKABLE void fetchUsers() { sendRequest("user.list", QJsonObject{}); }

    Q_INVOKABLE void borrowBook(const QString &bookId, const QString &userId) {
        QJsonObject payload;
        payload["id"] = bookId;
        payload["user_id"] = userId;
        sendRequest("book.borrow", payload);
    }

    Q_INVOKABLE void returnBook(const QString &bookId) {
        QJsonObject payload;
        payload["id"] = bookId;
        sendRequest("book.return", payload);
    }

    Q_INVOKABLE void addBook(const QString &t, const QString &a, int y, const QString &g, int p, int w, const QString &l) {
        QJsonObject pl;
        pl["title"] = t;
        pl["authors"] = QJsonArray{a};
        pl["year"] = y;
        pl["genre"] = g;
        pl["pages"] = p;
        pl["weight"] = w;
        pl["location"] = l;
        sendRequest("book.add", pl);
    }

    Q_INVOKABLE void updateBook(const QString &id, const QString &t, const QString &a, int y, const QString &g, int p, int w, const QString &l) {
        QJsonObject pl;
        pl["id"] = id;
        pl["title"] = t;
        pl["authors"] = QJsonArray{a};
        pl["year"] = y;
        pl["genre"] = g;
        pl["pages"] = p;
        pl["weight"] = w;
        pl["location"] = l;
        sendRequest("book.update", pl);
    }

    Q_INVOKABLE void deleteBook(const QString &id) {
        sendRequest("book.remove", QJsonObject{{"id", id}}, "del-" + id);
    }

signals:
    void connectedChanged();
    void tokenChanged();
    void authSuccess();
    void authError(const QString &message);
    void connectionError(const QString &message);
    void booksReceived(const QJsonArray &books);
    void usersReceived(const QJsonArray &users);
    void bookUpdated(const QJsonObject &book);
    void bookDeleted(const QString &id);
    void recentLibrariesChanged();

private:
    void sendRequest(const QString &action, const QJsonObject &payload, const QString &customId = "") {
        if (!m_ws || m_ws->state() != QAbstractSocket::ConnectedState) return;
        QJsonObject msg;
        msg["type"] = "request";
        msg["id"] = customId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : customId;
        msg["action"] = action;
        msg["payload"] = payload;
        if (!m_token.isEmpty()) msg["token"] = m_token;
        m_ws->sendTextMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    }

    void saveRecent(const QString &host, int port, const QString &code, const QString &user) {
        QSettings settings("LibSorter", "Client");
        QVariantList list = settings.value("recent").toList();
        for (int i = 0; i < list.size(); ++i) {
            QVariantMap m = list[i].toMap();
            if (m["host"] == host && m["code"] == code) {
                list.removeAt(i);
                break;
            }
        }
        QVariantMap entry;
        entry["host"] = host;
        entry["port"] = port;
        entry["code"] = code;
        entry["username"] = user;
        list.prepend(entry);
        if (list.size() > 5) list.removeLast();
        settings.setValue("recent", list);
        m_recent = list;
        emit recentLibrariesChanged();
    }

    void loadRecent() {
        QSettings settings("LibSorter", "Client");
        m_recent = settings.value("recent").toList();
    }

    QWebSocket *m_ws = nullptr;
    bool m_connected = false;
    QString m_token;
    QVariantList m_recent;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    LibSorterClient client;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("libClient", &client);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("LibSorter", "Main");
    return app.exec();
}
