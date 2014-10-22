#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QColorDialog>

#include "NifHeaders.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void LoadMesh(const QString & name);
    void LoadLayers(Niflib::NiTriShapeRef triShape);
    void SaveLayers(Niflib::NiTriShapeRef triShape);
    void ClearLayers();

    Niflib::NifInfo m_info;
    Niflib::NiObjectRef m_rootNode;

public slots:
    void on_addLayerButton_clicked();
    void on_removeLayerButton_clicked();
    void onCopyToClipboard();
    void onPasteFromClipboard();

private slots:
    void on_layerView_itemDoubleClicked(QTableWidgetItem *item);
    void on_heightComboBox_currentTextChanged(const QString &arg1);
    void on_widthComboBox_currentTextChanged(const QString &arg1);
    void on_lockResolution_toggled(bool checked);
    void on_applyLayersButton_clicked();
    void on_nodeView_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_nifLoadButton_clicked();
    void on_nifSaveButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
