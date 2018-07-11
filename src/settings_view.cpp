/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <qcoreapplication.h>

#include "settings.h"

#include "settings_view.h"

SettingsView::SettingsView(QWidget *parent)
 : QDialog(parent)
{
    setWindowTitle("settings");

    setModal(true);

    QPushButton *b_select_dir=new QPushButton("select dir");
    connect(b_select_dir, SIGNAL(clicked(bool)), SLOT(selectLibraryPath()));

    QLabel *l_library_path=new QLabel("library path:");

    QLabel *l_file_filter=new QLabel("file filter:");

    le_library_path=new QLineEdit();
    le_file_filter=new QLineEdit();

    cb_filter_empty_dirs=new QCheckBox("filter empty dirs");

    QGridLayout *la_controls=new QGridLayout();

    int row=0;

    la_controls->addWidget(l_library_path, row, 0);
    la_controls->addWidget(le_library_path, row, 1);
    la_controls->addWidget(b_select_dir, row, 2);

    row++;

    la_controls->addWidget(l_file_filter, row, 0);
    la_controls->addWidget(le_file_filter, row, 1);

    row++;

    la_controls->addWidget(cb_filter_empty_dirs, row, 1);

    //

    QPushButton *b_ok=new QPushButton("ok");
    connect(b_ok, SIGNAL(clicked(bool)), SLOT(accept()));

    QPushButton *b_cancel=new QPushButton("cancel");
    connect(b_cancel, SIGNAL(clicked(bool)), SLOT(reject()));

    QHBoxLayout *la_buttons=new QHBoxLayout();
    la_buttons->addStretch(1);
    la_buttons->addWidget(b_ok);
    la_buttons->addWidget(b_cancel);
    la_buttons->addStretch(1);

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addLayout(la_controls);
    la_main->addLayout(la_buttons);

    setLayout(la_main);

    connect(this, SIGNAL(accepted()), SLOT(updateSettings()));
}

int SettingsView::exec()
{
    le_library_path->setText(settings->main.library_path);
    le_file_filter->setText(settings->main.file_filter);
    cb_filter_empty_dirs->setChecked(settings->main.filter_empty_dirs);

    return QDialog::exec();
}

void SettingsView::updateSettings()
{
    settings->main.library_path=le_library_path->text().simplified();
    settings->main.file_filter=le_file_filter->text().simplified();
    settings->main.filter_empty_dirs=cb_filter_empty_dirs->isChecked();

    emit settings->updated();
}

void SettingsView::selectLibraryPath()
{
    QString path=QFileDialog::getExistingDirectory(this, "select library directory", le_library_path->text().simplified());

    if(path.isEmpty())
        return;

    le_library_path->setText(path);
}
