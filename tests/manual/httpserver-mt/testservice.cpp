/*
 * Copyright (C) 2012-2013 Matt Broadstone
 * Contact: http://bitbucket.org/devonit/qjsonrpc
 *
 * This file is part of the QJsonRpc Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QThreadPool>
#include <QRunnable>
#include <QHostAddress>

#include "testservice.h"
#include <QTcpSocket>
#include "libQJsonRPC/qjsonrpcsocket.h"

TestService::TestService(QObject *parent)
    : QJsonRpcService(parent)
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
}

void TestService::testMethod()
{
    qDebug() << Q_FUNC_INFO << "called" << endl;
}

void TestService::testMethodWithParams(const QString &first, bool second, double third)
{
    qDebug()<<this->currentRequest().socket()->device();

    QTcpSocket* Socket = qobject_cast<QTcpSocket*>(this->currentRequest().socket()->device());
    if (!Socket)
        qCritical("Invalid socket type");
    qDebug()<<Socket;
    if (Socket)
        qDebug()<<Socket->peerAddress()<<Socket->peerName();
    qDebug() << Q_FUNC_INFO << "called with parameters: " << endl
             << " first: " << first << endl
             << "second: " << second << endl
             << " third: " << third << endl;
}

void TestService::testMethodWithVariantParams(const QString &first, bool second, double third, const QVariant &fourth)
{
    qDebug() << Q_FUNC_INFO << "called with variant parameters: " << endl
             << " first: " << first << endl
             << "second: " << second << endl
             << " third: " << third << endl
             << "fourth: " << fourth << endl;
}

QString TestService::testMethodWithParamsAndReturnValue(const QString &name)
{
    return QString("Hello %1").arg(name);
}

void TestService::testMethodWithDefaultParameter(const QString &first, const QString &second)
{
    qDebug() << Q_FUNC_INFO << endl
             << "first: " << first << endl
             << (second.isEmpty() ? "not defined, default parameter" : second) << endl;
}

QString TestService::immediateResponse()
{
    return "immediate";
}

QString TestService::longTaskWithImmediateResponse()
{
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    return "long immediate";
}

class DelayedResponseJob : public QRunnable
{
public:
    DelayedResponseJob(const QJsonRpcServiceRequest &request)
        : m_request(request)
    {
    }

protected:
    virtual void run() {
        // do some work
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

        // respond
        QJsonRpcMessage response = m_request.request().createResponse(QLatin1String("long delayed"));
        m_request.respond(response);
    }

private:
    QJsonRpcServiceRequest m_request;

};

QString TestService::longTaskWithDelayedResponse()
{
    beginDelayedResponse();
    QThreadPool::globalInstance()->start(new DelayedResponseJob(currentRequest()));
    return QString();
}