#include "control.h"

#include "core/exception/invalid_state_exception.h"
#include "file_dialogs.h"
#include "ui_control.h"

const QString BasicImageViewControl::invalidDialogType =
    QString("DialogType::SELECT_FILE=%1, DialogType::SELECT_FOLDER=%2")
        .arg(static_cast<int>(DialogType::SELECT_FILE))
        .arg(static_cast<int>(DialogType::SELECT_FOLDER));

BasicImageViewControl::BasicImageViewControl(QWidget *parent)
    : QWidget(parent), ui(new Ui::BasicImageViewControl)
{
    ui->setupUi(this);

    connect(ui->resetButton, &QPushButton::clicked, this,
            &BasicImageViewControl::resetButtonClicked);
    connect(ui->saveButton, &QPushButton::clicked, this,
            &BasicImageViewControl::onSaveButtonClicked);
    connect(ui->loadButton, &QPushButton::clicked, this,
            &BasicImageViewControl::onLoadButtonClicked);
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

void BasicImageViewControl::onLoadButtonClicked()
{
    ImageOpenDialog dialog;
    connect(&dialog, &ImageOpenDialog::fileSelected, this,
            &BasicImageViewControl::loadFileSelected);
    connect(&dialog, &ImageOpenDialog::fileSelected, this,
            &BasicImageViewControl::onLoadFileSelected);

    dialog.exec();
}

void BasicImageViewControl::onLoadFileSelected(const QString &path)
{
    fileName = path;
}
