#include "SingleInstance.h"

#include <QDir>
#include <QElapsedTimer>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>
#include <QThread>

#include <algorithm>
#include <utility>

namespace strikepro {

SingleInstance::SingleInstance(QString serverName, QObject *parent)
    : QObject(parent)
    , m_serverName(std::move(serverName))
    , m_server(new QLocalServer(this))
{
    QString runtimeDirectory =
        QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runtimeDirectory.isEmpty()) {
        runtimeDirectory =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    m_lockFile = std::make_unique<QLockFile>(
        QDir(runtimeDirectory).filePath(m_serverName + QStringLiteral(".lock")));
    connect(
        m_server,
        &QLocalServer::newConnection,
        this,
        &SingleInstance::acceptConnections);
}

SingleInstance::~SingleInstance()
{
    if (!m_primary) {
        return;
    }
    m_server->close();
    QLocalServer::removeServer(m_serverName);
    m_lockFile->unlock();
}

SingleInstance::Role SingleInstance::start(QString *error)
{
    if (m_primary) {
        return Role::Primary;
    }

    if (!m_lockFile->tryLock()) {
        if (notifyPrimary()) {
            return Role::Secondary;
        }
        if (error != nullptr) {
            *error = QStringLiteral(
                "Another instance is running but did not accept activation.");
        }
        return Role::Error;
    }

    QLocalServer::removeServer(m_serverName);
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!m_server->listen(m_serverName)) {
        if (error != nullptr) {
            *error = m_server->errorString();
        }
        m_lockFile->unlock();
        return Role::Error;
    }

    m_primary = true;
    return Role::Primary;
}

bool SingleInstance::notifyPrimary(const int timeoutMs) const
{
    QElapsedTimer timer;
    timer.start();
    do {
        QLocalSocket socket;
        socket.connectToServer(m_serverName, QIODevice::WriteOnly);
        const int remaining = std::max(1, timeoutMs - int(timer.elapsed()));
        if (socket.waitForConnected(std::min(remaining, 100))) {
            socket.write("activate\n");
            socket.waitForBytesWritten(std::min(remaining, 100));
            socket.disconnectFromServer();
            return true;
        }
        QThread::msleep(25);
    } while (timer.elapsed() < timeoutMs);
    return false;
}

void SingleInstance::acceptConnections()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        if (socket == nullptr) {
            continue;
        }
        socket->write("ok\n");
        socket->flush();
        socket->disconnectFromServer();
        socket->deleteLater();
        emit activationRequested();
    }
}

} // namespace strikepro
