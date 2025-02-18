// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/uniqueobjectptr.h>

#include <QDialog>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QTableView;
QT_END_NAMESPACE

namespace Utils {
class FancyLineEdit;
}

namespace QmlDesigner {

class SignalListDialog : public QDialog
{
    Q_OBJECT

public:
    SignalListDialog(QWidget *parent = nullptr);
    ~SignalListDialog() override;

    void initialize(QStandardItemModel *model);

    class SignalListDelegate *signalListDelegate() const;

private:
    Utils::UniqueObjectPtr<SignalListDelegate> m_signalListDelegate;
    Utils::UniqueObjectPtr<QTableView> m_table;
    Utils::UniqueObjectPtr<Utils::FancyLineEdit> m_searchLine;
};

} // QmlDesigner namespace
