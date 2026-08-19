#include "control.h"

#include "features/framework/core/exception/invalid_state_exception.h"
#include "features/framework/gui/design_system.h"
#include "file_dialogs.h"

#include <QHBoxLayout>
#include <QPushButton>

const QString BasicImageViewControl::invalidDialogType =
    QString("DialogType::SELECT_FILE=%1, DialogType::SELECT_FOLDER=%2")
        .arg(static_cast<int>(DialogType::SELECT_FILE))
        .arg(static_cast<int>(DialogType::SELECT_FOLDER));

BasicImageViewControl::BasicImageViewControl(QWidget *parent)
    : QWidget(parent), ui(new Ui::BasicImageViewControl)
{
    ui->loadButton = new QPushButton(tr("Load"), this);
    DevTools::Ui::configureCompactButton(ui->loadButton);
    ui->resetButton = new QPushButton(tr("Reset"), this);
    DevTools::Ui::configureCompactButton(ui->resetButton);
    ui->saveButton = new QPushButton(tr("Save"), this);
    DevTools::Ui::configureCompactButton(ui->saveButton);

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(ui->loadButton);
    layout->addWidget(ui->resetButton);
    layout->addWidget(ui->saveButton);
    DevTools::Ui::configureActionBar(layout, DevTools::Ui::ActionBarAlignment::Trailing);

    connect(ui->resetButton, &QPushButton::clicked, this,
            &BasicImageViewControl::resetButtonClicked);
    connect(ui->saveButton, &QPushButton::clicked, this,
            &BasicImageViewControl::onSaveButtonClicked);
    connect(ui->loadButton, &QPushButton::clicked, this,
            &BasicImageViewControl::onLoadButtonClicked);
    connect(this, &BasicImageViewControl::loadFileSelected, this,
            &BasicImageViewControl::onLoadFileSelected);
}

BasicImageViewControl::~BasicImageViewControl()
{
    delete ui;
}

void BasicImageViewControl::onSaveButtonClicked()
{
    switch (saveFileDialogType) {
    case DialogType::SELECT_FILE: {
        ImageSaveDialog dialog;

        if (fileName.endsWith(".svg")) {
            // svg形式はpngにして保存する
            dialog.selectFile(fileName.replace(fileName.length() - 3, 3, "png"));
        } else {
            dialog.selectFile(fileName);
        }

        connect(&dialog, &ImageSaveDialog::fileSelected, this,
                &BasicImageViewControl::saveFileSelected);

        dialog.exec();
    } break;
    case DialogType::SELECT_FOLDER: {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setLabelText(QFileDialog::Accept, tr("Save"));
        connect(&dialog, &QFileDialog::fileSelected, this,
                &BasicImageViewControl::saveFileSelected);
        dialog.exec();
    } break;
    default:
        throw InvalidStateException(QString::number(static_cast<int>(saveFileDialogType)),
                                    invalidDialogType);
        break;
    }
}

void BasicImageViewControl::onLoadButtonClicked() const
{
    ImageOpenDialog dialog;
    connect(&dialog, &ImageOpenDialog::fileSelected, this,
            &BasicImageViewControl::loadFileSelected);

    dialog.exec();
}

void BasicImageViewControl::onLoadFileSelected(const QString &path)
{
    fileName = path;
}
