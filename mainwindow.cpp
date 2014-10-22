#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "nitrishapewidget.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QShortcut>
#include <QClipboard>
#include <QMimeData>
#include <QDataStream>

QImage readDDSFile(const QString &filename);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(new QShortcut(QKeySequence("Ins"), this), SIGNAL(activated()), this, SLOT(on_addLayerButton_clicked()));
    QObject::connect(new QShortcut(QKeySequence("Del"), this), SIGNAL(activated()), this, SLOT(on_removeLayerButton_clicked()));
    QObject::connect(new QShortcut(QKeySequence("Ctrl+C"), this), SIGNAL(activated()), this, SLOT(onCopyToClipboard()));
    QObject::connect(new QShortcut(QKeySequence("Ctrl+V"), this), SIGNAL(activated()), this, SLOT(onPasteFromClipboard()));
}

MainWindow::~MainWindow()
{
    if(m_rootNode)
        m_rootNode->SubtractRef();

    delete ui;
}

void MainWindow::LoadMesh(const QString & name)
{
    if(m_rootNode) {
        ui->nodeView->clear();
        m_rootNode->SubtractRef();
    }

    QFileInfo info(name);
    if(!info.exists())
        return;

    m_rootNode = ReadNifTree( name.toStdString().c_str(), &m_info );
    if(m_rootNode) {
        m_rootNode->AddRef();
        auto node = Niflib::DynamicCast<Niflib::NiNode>( m_rootNode );
        if(node) {
            auto children = node->GetChildren();
            for(auto child : children) {
                auto triShape = Niflib::DynamicCast<Niflib::NiTriShape>(child);
                if(triShape) {
                    new NiTriShapeWidget(triShape, ui->nodeView);
                }
            }
        }
    }
}

void MainWindow::LoadLayers(Niflib::NiTriShapeRef triShape)
{
    Niflib::Ref<Niflib::NiStringsExtraData> textures;
    Niflib::Ref<Niflib::NiIntegersExtraData> colors;
    Niflib::Ref<Niflib::NiFloatsExtraData> alphas;
    Niflib::Ref<Niflib::NiIntegerExtraData> resolution;
    Niflib::Ref<Niflib::NiIntegerExtraData> resolutionW;
    Niflib::Ref<Niflib::NiIntegerExtraData> resolutionH;

    auto dataList = triShape->GetExtraData();
    for(auto extraData : dataList) {
        if(extraData->GetName() == "MASKT")
            textures = Niflib::DynamicCast<Niflib::NiStringsExtraData>(extraData);
        if(extraData->GetName() == "MASKC")
            colors = Niflib::DynamicCast<Niflib::NiIntegersExtraData>(extraData);
        if(extraData->GetName() == "MASKA")
            alphas = Niflib::DynamicCast<Niflib::NiFloatsExtraData>(extraData);
        if(extraData->GetName() == "MASKR")
            resolution = Niflib::DynamicCast<Niflib::NiIntegerExtraData>(extraData);
        if(extraData->GetName() == "MASKW")
            resolutionW = Niflib::DynamicCast<Niflib::NiIntegerExtraData>(extraData);
        if(extraData->GetName() == "MASKH")
            resolutionH = Niflib::DynamicCast<Niflib::NiIntegerExtraData>(extraData);
    }

    if(textures) {
        auto textureData = textures->GetData();
        ui->layerView->setRowCount(textureData.size());
        for(quint32 i = 0; i < textureData.size(); i++) {
            if(i >= 15)
                break;
            auto str = textureData.at(i);
            auto widgetItem = new QTableWidgetItem(str.c_str(), 0);
            ui->layerView->setItem(i, 0, widgetItem);
        }

        if(colors) {
            auto colorData = colors->GetData();
            for(quint32 i = 0; i < colorData.size(); i++) {
                if(i >= 15)
                    break;
                auto colorValue = colorData.at(i);
                float alpha = 1.0;
                if(alphas) {
                    auto alphaData = alphas->GetData();
                    if(alphaData.size() <= colorData.size())
                        alpha = alphaData.at(i);
                }
                auto widgetItem = new QTableWidgetItem(1);
                QColor color = colorValue;
                color.setAlphaF(alpha);
                widgetItem->setBackgroundColor(color);
                widgetItem->setFlags(widgetItem->flags() & ~Qt::ItemIsEditable);
                ui->layerView->setItem(i, 1, widgetItem);
            }
        }

        if(resolution) {
            ui->heightComboBox->setCurrentText(QString("%1").arg(resolution->GetData()));
            ui->widthComboBox->setCurrentText(QString("%1").arg(resolution->GetData()));
            ui->lockResolution->setChecked(true);
        }
        if(resolutionH) {
            ui->widthComboBox->setCurrentText(QString("%1").arg(resolutionH->GetData()));
            ui->lockResolution->setChecked(false);
        }
        if(resolutionW) {
            ui->widthComboBox->setCurrentText(QString("%1").arg(resolutionW->GetData()));
            ui->lockResolution->setChecked(false);
        }
    }
}

void MainWindow::SaveLayers(Niflib::NiTriShapeRef triShape)
{
    auto dataList = triShape->GetExtraData();
    for(auto extraData : dataList) {
        if(extraData->GetName() == "MASKT" ||
           extraData->GetName() == "MASKC" ||
           extraData->GetName() == "MASKA" ||
           extraData->GetName() == "MASKR" ||
           extraData->GetName() == "MASKW" ||
           extraData->GetName() == "MASKH")
            triShape->RemoveExtraData(extraData);
    }

    Niflib::NiStringsExtraData * textures = NULL;
    Niflib::NiIntegersExtraData * colors = NULL;
    Niflib::NiFloatsExtraData * alphas = NULL;
    Niflib::NiIntegerExtraData * resolution = NULL;
    Niflib::NiIntegerExtraData * resolutionW = NULL;
    Niflib::NiIntegerExtraData * resolutionH = NULL;

    if(ui->layerView->rowCount() > 0) {
        textures = new Niflib::NiStringsExtraData();
        textures->SetName("MASKT");
        colors = new Niflib::NiIntegersExtraData();
        colors->SetName("MASKC");
        alphas = new Niflib::NiFloatsExtraData();
        alphas->SetName("MASKA");
        std::vector<std::string> strList;
        std::vector<unsigned int> colorList;
        std::vector<float> alphaList;
        for(qint32 i = 0; i < ui->layerView->rowCount(); i++) {
            auto textureItem = ui->layerView->item(i, 0);
            auto colorItem = ui->layerView->item(i, 1);
            QColor colorValue = colorItem->backgroundColor();
            colorList.push_back(colorValue.rgb());
            alphaList.push_back(colorValue.alphaF());
            strList.push_back(textureItem->text().toStdString());
        }
        textures->SetData(strList);
        colors->SetData(colorList);
        alphas->SetData(alphaList);

        triShape->AddExtraData(textures);
        triShape->AddExtraData(colors);
        triShape->AddExtraData(alphas);

        if(ui->widthComboBox->currentText() == ui->heightComboBox->currentText()) {
            resolution = new Niflib::NiIntegerExtraData();
            resolution->SetName("MASKR");
            resolution->SetData(ui->widthComboBox->currentText().toInt());
            triShape->AddExtraData(resolution);
        } else {
            resolutionW = new Niflib::NiIntegerExtraData();
            resolutionW->SetName("MASKW");
            resolutionW->SetData(ui->widthComboBox->currentText().toInt());
            resolutionH = new Niflib::NiIntegerExtraData();
            resolutionH->SetName("MASKH");
            resolutionH->SetData(ui->heightComboBox->currentText().toInt());

            triShape->AddExtraData(resolutionW);
            triShape->AddExtraData(resolutionH);
        }
    }
}

void MainWindow::on_layerView_itemDoubleClicked(QTableWidgetItem *item)
{
    if(item->type() == 1) {
        QColor newColor = QColorDialog::getColor(item->backgroundColor(), this, tr("Select Color"), QColorDialog::ShowAlphaChannel);
        if(newColor.isValid()) {
            item->setBackgroundColor(newColor);
        }
    }
}

void MainWindow::on_heightComboBox_currentTextChanged(const QString &arg1)
{
    if(ui->lockResolution->isChecked()) {
        ui->widthComboBox->setCurrentText(arg1);
    }
}

void MainWindow::on_widthComboBox_currentTextChanged(const QString &arg1)
{
    if(ui->lockResolution->isChecked()) {
        ui->heightComboBox->setCurrentText(arg1);
    }
}

void MainWindow::on_lockResolution_toggled(bool checked)
{
    if(checked) {
        ui->widthComboBox->setCurrentText(ui->heightComboBox->currentText());
    }
}

void MainWindow::on_addLayerButton_clicked()
{
    if(!ui->nodeView->currentItem())
        return;

    quint32 row = ui->layerView->rowCount();
    if(row == 14)
        return;

    auto textureItem = new QTableWidgetItem(0);
    textureItem->setText(tr("textures\\tintmasks\\default.dds"));

    auto colorItem = new QTableWidgetItem(1);
    QColor color = 0x797979;
    colorItem->setBackgroundColor(color);
    colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);

    ui->layerView->insertRow(row);
    ui->layerView->setItem(row, 0, textureItem);
    ui->layerView->setItem(row, 1, colorItem);
}

void MainWindow::on_removeLayerButton_clicked()
{
    if(!ui->nodeView->currentItem())
        return;

    ui->layerView->removeRow(ui->layerView->currentRow());
}

void MainWindow::on_applyLayersButton_clicked()
{
    NiTriShapeWidget * trishapeWidget = dynamic_cast<NiTriShapeWidget*>(ui->nodeView->currentItem());
    if(trishapeWidget) {
        SaveLayers(trishapeWidget->GetNiTriShape());
    }
}

void MainWindow::ClearLayers()
{
    ui->layerView->setRowCount(0);
    ui->widthComboBox->setCurrentText("128");
    ui->heightComboBox->setCurrentText("128");
    ui->lockResolution->setChecked(false);
}

void MainWindow::on_nodeView_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ClearLayers();
    NiTriShapeWidget * trishapeWidget = dynamic_cast<NiTriShapeWidget*>(current);
    if(trishapeWidget) {
        auto triShape = trishapeWidget->GetNiTriShape();
        if(triShape) {
            LoadLayers(triShape);
        }
    }
}

void MainWindow::on_nifLoadButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Nif"), ui->nifLoadPathEdit->text(), tr("NetImmerse File (*.nif);;All Files (*)"));
    if(!filePath.isEmpty()) {
        ui->nifLoadPathEdit->setText(filePath);
        ui->nifSavePathEdit->setText(filePath);
        LoadMesh(ui->nifLoadPathEdit->text());
    }
}

void MainWindow::on_nifSaveButton_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Select Nif"), ui->nifLoadPathEdit->text(), tr("NetImmerse File (*.nif);;All Files (*)"));
    if(!filePath.isEmpty()) {
        ui->nifSavePathEdit->setText(filePath);
        if(!ui->nifSavePathEdit->text().isEmpty() && m_rootNode) {
            on_applyLayersButton_clicked();
            WriteNifTree(ui->nifSavePathEdit->text().toStdString(), m_rootNode, m_info);
        }
    }
}

void MainWindow::onCopyToClipboard()
{
    if(!ui->nodeView->currentItem())
        return;

    QStringList textureList;
    QList<QColor> colorList;

    for(qint32 i = 0; i < ui->layerView->rowCount(); i++) {
        auto textureItem = ui->layerView->item(i, 0);
        auto colorItem = ui->layerView->item(i, 1);

        QColor colorValue = colorItem->backgroundColor();

        textureList.push_back(textureItem->text());
        colorList.push_back(colorValue);
    }

    QClipboard *clipboard = QApplication::clipboard();
    if(!textureList.isEmpty() && !colorList.isEmpty()) {
        QMimeData * mimeData = new QMimeData();
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << textureList;
        stream << colorList;
        stream << ui->widthComboBox->currentText();
        stream << ui->heightComboBox->currentText();
        mimeData->setData("application/dyeEditor", data);
        clipboard->setMimeData(mimeData);
        ui->statusBar->showMessage(tr("Copied %1 layers to clipboard.").arg(textureList.size()), 2000);
    }
}

void MainWindow::onPasteFromClipboard()
{
    if(!ui->nodeView->currentItem())
        return;

    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData * mimeData = clipboard->mimeData();
    if(mimeData) {
        QByteArray data = mimeData->data("application/dyeEditor");
        if(!data.isEmpty()) {
            QStringList textureList;
            QList<QColor> colorList;
            QDataStream stream(&data, QIODevice::ReadOnly);
            stream >> textureList;
            stream >> colorList;
            QString width, height;
            stream >> width;
            stream >> height;

            ClearLayers();

            ui->layerView->setRowCount(textureList.size());
            for(qint32 i = 0; i < textureList.size(); i++)
            {
                auto textureItem = new QTableWidgetItem(0);
                textureItem->setText(textureList.at(i));

                auto colorItem = new QTableWidgetItem(1);
                QColor color = colorList.at(i);
                colorItem->setBackgroundColor(color);
                colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);

                ui->layerView->setItem(i, 0, textureItem);
                ui->layerView->setItem(i, 1, colorItem);
            }

            ui->widthComboBox->setCurrentText(width);
            ui->heightComboBox->setCurrentText(height);
            ui->lockResolution->setChecked(width == height);
            ui->statusBar->showMessage(tr("Pasted %1 layers to clipboard.").arg(textureList.size()), 2000);
        }
    }
}
