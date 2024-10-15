#include "control.h"
#include "ui_control.h"

#include "file_dialogs.h"

BasicImageViewControl::BasicImageViewControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BasicImageViewControl)
{
    ui->setupUi(this);

    connect(ui->resetButton,
            &QPushButton::clicked,
            this,
            &BasicImageViewControl::resetButtonClicked);

    connect(ui->saveButton,
            &QPushButton::clicked,
            this,
            &BasicImageViewControl::onSaveButtonClicked);
    connect(ui->loadButton,
            &QPushButton::clicked,
            this,
            &BasicImageViewControl::onLoadButtonClicked);
}

BasicImageViewControl::~BasicImageViewControl()
{
    delete ui;
}

void BasicImageViewControl::onSaveButtonClicked()
{
    ImageSaveDialog dialog;
    connect(&dialog, &ImageSaveDialog::fileSelected, this, &BasicImageViewControl::saveFileSelected);

    dialog.exec();
}

void BasicImageViewControl::onLoadButtonClicked()
{
    ImageOpenDialog dialog;
    connect(&dialog, &ImageOpenDialog::fileSelected, this, &BasicImageViewControl::loadFileSelected);

    dialog.exec();
}
