// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "protocol.h"

#include "cpastertr.h"

#include <coreplugin/icore.h>
#include <coreplugin/dialogs/ioptionspage.h>

#include <utils/networkaccessmanager.h>
#include <utils/mimeconstants.h>

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QUrl>
#include <QVariant>

#include <memory>

namespace CodePaster {

Protocol::Protocol()
        : QObject()
{
}

Protocol::~Protocol() = default;

bool Protocol::hasSettings() const
{
    return false;
}

bool Protocol::checkConfiguration(QString *)
{
    return true;
}

const Core::IOptionsPage *Protocol::settingsPage() const
{
    return nullptr;
}

void Protocol::list()
{
    qFatal("Base Protocol list() called");
}

Protocol::ContentType Protocol::contentType(const QString &mt)
{
    using namespace Utils::Constants;
    if (mt == QLatin1String(C_SOURCE_MIMETYPE)
        || mt == QLatin1String(C_HEADER_MIMETYPE)
        || mt == QLatin1String(GLSL_MIMETYPE)
        || mt == QLatin1String(GLSL_VERT_MIMETYPE)
        || mt == QLatin1String(GLSL_FRAG_MIMETYPE)
        || mt == QLatin1String(GLSL_ES_VERT_MIMETYPE)
        || mt == QLatin1String(GLSL_ES_FRAG_MIMETYPE))
        return C;
    if (mt == QLatin1String(CPP_SOURCE_MIMETYPE)
        || mt == QLatin1String(CPP_HEADER_MIMETYPE)
        || mt == QLatin1String(OBJECTIVE_C_SOURCE_MIMETYPE)
        || mt == QLatin1String(OBJECTIVE_CPP_SOURCE_MIMETYPE))
        return Cpp;
    if (mt == QLatin1String(QML_MIMETYPE)
        || mt == QLatin1String(QMLUI_MIMETYPE)
        || mt == QLatin1String(QMLPROJECT_MIMETYPE)
        || mt == QLatin1String(QBS_MIMETYPE)
        || mt == QLatin1String(JS_MIMETYPE)
        || mt == QLatin1String(JSON_MIMETYPE))
        return JavaScript;
    if (mt == QLatin1String("text/x-patch"))
        return Diff;
    if (mt == QLatin1String("text/xml")
        || mt == QLatin1String("application/xml")
        || mt == QLatin1String(RESOURCE_MIMETYPE)
        || mt == QLatin1String(FORM_MIMETYPE))
        return Xml;
    return Text;
}

QString Protocol::fixNewLines(QString data)
{
    // Copied from cpaster. Otherwise lineendings will screw up
    // HTML requires "\r\n".
    if (data.contains(QLatin1String("\r\n")))
        return data;
    if (data.contains(QLatin1Char('\n'))) {
        data.replace(QLatin1Char('\n'), QLatin1String("\r\n"));
        return data;
    }
    if (data.contains(QLatin1Char('\r')))
        data.replace(QLatin1Char('\r'), QLatin1String("\r\n"));
    return data;
}

QString Protocol::textFromHtml(QString data)
{
    data.remove(QLatin1Char('\r'));
    data.replace(QLatin1String("&lt;"), QString(QLatin1Char('<')));
    data.replace(QLatin1String("&gt;"), QString(QLatin1Char('>')));
    data.replace(QLatin1String("&amp;"), QString(QLatin1Char('&')));
    data.replace(QLatin1String("&quot;"), QString(QLatin1Char('"')));
    return data;
}

bool Protocol::ensureConfiguration(Protocol *p, QWidget *parent)
{
    QString errorMessage;
    bool ok = false;
    while (true) {
        if (p->checkConfiguration(&errorMessage)) {
            ok = true;
            break;
        }
        // Cancel returns empty error message.
        if (errorMessage.isEmpty() || !showConfigurationError(p, errorMessage, parent))
            break;
    }
    return ok;
}

bool Protocol::showConfigurationError(const Protocol *p,
                                      const QString &message,
                                      QWidget *parent,
                                      bool showConfig)
{
    if (!p->settingsPage())
        showConfig = false;

    if (!parent)
        parent = Core::ICore::dialogParent();
    const QString title = Tr::tr("%1 - Configuration Error").arg(p->name());
    QMessageBox mb(QMessageBox::Warning, title, message, QMessageBox::Cancel, parent);
    QPushButton *settingsButton = nullptr;
    if (showConfig)
        settingsButton = mb.addButton(Core::ICore::msgShowOptionsDialog(), QMessageBox::AcceptRole);
    mb.exec();
    bool rc = false;
    if (mb.clickedButton() == settingsButton)
        rc = Core::ICore::showOptionsDialog(p->settingsPage()->id(), parent);
    return rc;
}

// --------- NetworkProtocol

static void addCookies(QNetworkRequest &request)
{
    auto accessMgr = Utils::NetworkAccessManager::instance();
    const QList<QNetworkCookie> cookies = accessMgr->cookieJar()->cookiesForUrl(request.url());
    for (const QNetworkCookie &cookie : cookies)
        request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookie));
}

QNetworkReply *NetworkProtocol::httpGet(const QString &link, bool handleCookies)
{
    QUrl url(link);
    QNetworkRequest r(url);
    if (handleCookies)
        addCookies(r);
    return Utils::NetworkAccessManager::instance()->get(r);
}

QNetworkReply *NetworkProtocol::httpPost(const QString &link, const QByteArray &data,
                                         bool handleCookies)
{
    QUrl url(link);
    QNetworkRequest r(url);
    if (handleCookies)
        addCookies(r);
    r.setHeader(QNetworkRequest::ContentTypeHeader,
                QVariant(QByteArray("application/x-www-form-urlencoded")));
    return Utils::NetworkAccessManager::instance()->post(r, data);
}

NetworkProtocol::~NetworkProtocol() = default;

bool NetworkProtocol::httpStatus(QString url, QString *errorMessage, bool useHttps)
{
    // Connect to host and display a message box, using its event loop.
    errorMessage->clear();
    const QString httpPrefix = QLatin1String("http://");
    const QString httpsPrefix = QLatin1String("https://");
    if (!url.startsWith(httpPrefix) && !url.startsWith(httpsPrefix)) {
        url.prepend(useHttps ? httpsPrefix : httpPrefix);
        url.append(QLatin1Char('/'));
    }
    std::unique_ptr<QNetworkReply> reply(httpGet(url));
    QMessageBox box(QMessageBox::Information,
                    Tr::tr("Checking connection"),
                    Tr::tr("Connecting to %1...").arg(url),
                    QMessageBox::Cancel,
                    Core::ICore::dialogParent());
    connect(reply.get(), &QNetworkReply::finished, &box, &QWidget::close);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    box.exec();
    QApplication::restoreOverrideCursor();
    // User canceled, discard and be happy.
    if (!reply->isFinished()) {
        QNetworkReply *replyPtr = reply.release();
        connect(replyPtr, &QNetworkReply::finished, replyPtr, &QNetworkReply::deleteLater);
        return false;
    }
    // Passed
    if (reply->error() == QNetworkReply::NoError)
        return true;
    // Error.
    *errorMessage = reply->errorString();
    return false;
}

} // CodePaster
