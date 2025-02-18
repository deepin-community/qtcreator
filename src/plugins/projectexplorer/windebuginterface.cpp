// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "windebuginterface.h"

#ifdef Q_OS_WIN

#include <utils/qtcassert.h>
#include <utils/threadutils.h>
#include <QCoreApplication>
#include <qt_windows.h>

#include <algorithm>

/*!
    \class ProjectExplorer::Internal::WinDebugInterface
    \brief The WinDebugInterface class is used on Windows to capture output of
    the Windows API \c OutputDebugString() function.

    Emits output by process id.

    The \c OutputDebugString() function puts its data into a shared memory segment named
    \c DBWIN_BUFFER which can be accessed via file mapping.
*/

namespace ProjectExplorer {
namespace Internal {

WinDebugInterface *WinDebugInterface::m_instance = nullptr;

WinDebugInterface *WinDebugInterface::instance()
{
    return m_instance;
}

bool WinDebugInterface::stop()
{
    if (!m_instance->m_waitHandles[TerminateEventHandle])
        return false;
    SetEvent(m_instance->m_waitHandles[TerminateEventHandle]);
    return true;
}

WinDebugInterface::WinDebugInterface(QObject *parent) :
    QThread(parent)
{
    m_instance = this;
    m_creatorPid = QCoreApplication::applicationPid();
    setObjectName(QLatin1String("WinDebugInterfaceThread"));
    connect(this, &WinDebugInterface::_q_debugOutputReady,
            this, &WinDebugInterface::dispatchDebugOutput, Qt::QueuedConnection);
}

WinDebugInterface::~WinDebugInterface()
{
    if (stop())
        wait(500);
    m_instance = nullptr;
}

void WinDebugInterface::run()
{
    m_waitHandles[DataReadyEventHandle] = m_waitHandles[TerminateEventHandle] = nullptr;
    m_bufferReadyEvent = nullptr;
    m_sharedFile = nullptr;
    m_sharedMem  = nullptr;
    if (!runLoop())
        emit cannotRetrieveDebugOutput();
    if (m_sharedMem) {
        UnmapViewOfFile(m_sharedMem);
        m_sharedMem = nullptr;
    }
    if (m_sharedFile) {
        CloseHandle(m_sharedFile);
        m_sharedFile = nullptr;
    }
    if (m_waitHandles[TerminateEventHandle]) {
        CloseHandle(m_waitHandles[TerminateEventHandle]);
        m_waitHandles[TerminateEventHandle] = nullptr;
    }
    if (m_waitHandles[DataReadyEventHandle]) {
        CloseHandle(m_waitHandles[DataReadyEventHandle]);
        m_waitHandles[DataReadyEventHandle] = nullptr;
    }
    if (m_bufferReadyEvent) {
        CloseHandle(m_bufferReadyEvent);
        m_bufferReadyEvent = nullptr;
    }
}

bool WinDebugInterface::runLoop()
{
    m_waitHandles[TerminateEventHandle] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return false;
    m_waitHandles[DataReadyEventHandle] = CreateEvent(NULL, FALSE, FALSE, L"DBWIN_DATA_READY");
    if (!m_waitHandles[TerminateEventHandle] || !m_waitHandles[DataReadyEventHandle]
            || GetLastError() == ERROR_ALREADY_EXISTS)
        return false;
    m_bufferReadyEvent = CreateEvent(NULL, FALSE, FALSE, L"DBWIN_BUFFER_READY");
    if (!m_bufferReadyEvent
            || GetLastError() == ERROR_ALREADY_EXISTS)
        return false;
    m_sharedFile = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, 4096, L"DBWIN_BUFFER");
    if (!m_sharedFile || GetLastError() == ERROR_ALREADY_EXISTS)
        return false;
    m_sharedMem = MapViewOfFile(m_sharedFile, FILE_MAP_READ, 0, 0,  512);
    if (!m_sharedMem)
        return false;

    LPSTR  message = reinterpret_cast<LPSTR>(m_sharedMem) + sizeof(DWORD);
    auto processId = reinterpret_cast<LPDWORD>(m_sharedMem);

    SetEvent(m_bufferReadyEvent);

    while (true) {
        const DWORD ret = WaitForMultipleObjects(HandleCount, m_waitHandles, FALSE, INFINITE);
        if (ret == WAIT_FAILED || ret - WAIT_OBJECT_0 == TerminateEventHandle) {
            std::lock_guard<std::mutex> guard(m_outputMutex);
            emitReadySignal();
            break;
        }
        if (ret - WAIT_OBJECT_0 == DataReadyEventHandle) {
            if (*processId != m_creatorPid) {
                std::lock_guard<std::mutex> guard(m_outputMutex);
                m_debugOutput[*processId].push_back(QString::fromLocal8Bit(message));
                emitReadySignal();
            }
            SetEvent(m_bufferReadyEvent);
        }
    }
    return true;
}

void WinDebugInterface::emitReadySignal()
{
    // This function must be called from the WinDebugInterface thread only.
    QTC_ASSERT(QThread::currentThread() == this, return);

    if (m_debugOutput.empty() || m_readySignalEmitted)
        return;

    m_readySignalEmitted = true;
    emit _q_debugOutputReady();
}

void WinDebugInterface::dispatchDebugOutput()
{
    // Called in the thread this object was created in, not in the WinDebugInterfaceThread.
    QTC_ASSERT(Utils::isMainThread(), return);

    static size_t maxMessagesToSend = 100;
    std::map<qint64, QList<QString>> output;
    bool hasMoreOutput = false;

    m_outputMutex.lock();
    for (auto &entry : m_debugOutput) {
        auto it = entry.second.begin();
        for (; it != entry.second.end(); ++it) {
            output[entry.first].push_back(*it);
            if (output.size() >= maxMessagesToSend)
                break;
        }
        if (it != entry.second.begin())
            it = entry.second.erase(entry.second.begin(), it);
        if (it != entry.second.end()) {
            hasMoreOutput = true;
            break;
        }
    }
    if (!hasMoreOutput)
        m_readySignalEmitted = false;
    m_outputMutex.unlock();

    for (const auto &p : std::as_const(output))
        emit debugOutput(p.first, p.second);
    if (hasMoreOutput)
        emit _q_debugOutputReady();
}

void WinDebugInterface::startIfNeeded()
{
    if (!m_instance->isRunning())
        m_instance->start();
}

} // namespace Internal
} // namespace ProjectExplorer

#else

namespace ProjectExplorer {
namespace Internal {

WinDebugInterface *WinDebugInterface::m_instance = nullptr;

WinDebugInterface *WinDebugInterface::instance() { return nullptr; }

WinDebugInterface::WinDebugInterface(QObject *) { }

WinDebugInterface::~WinDebugInterface() { }

void WinDebugInterface::run() { }

bool WinDebugInterface::runLoop() { return false; }

void WinDebugInterface::emitReadySignal() { }

void WinDebugInterface::dispatchDebugOutput() { }

bool WinDebugInterface::stop() { return false; }

void WinDebugInterface::startIfNeeded() { }

} // namespace Internal
} // namespace ProjectExplorer

#endif
