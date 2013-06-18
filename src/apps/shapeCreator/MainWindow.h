#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "vtkCreator.h"
#include "creatorThread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
    void guiSettingsInit();
    void guiSettingsLoad();
    void guiSettingsSave();
    void guiFeaturesInit();

    void loadVtkFile(QString file);
	VtkCreator* vtkCreator();

	void msgOut(const QString &msg);
	void msgClearAll();

private:
    Ui::MainWindow *ui;
    QString _settingsFile;
    QString _lastPathVtk;
	boost::shared_ptr<CreateVolume> _pVtk;
	CreatorThread *_pThread;

protected:
    enum ShapeType
    {
        E_SHAPE_VTK = 1,
        E_SHAPE_CUBE,
        E_SHAPE_SCUBE,
        E_SHAPE_CONE,
        E_SHAPE_SPHERE,
        E_SHAPE_SSPHERE,
    };
private slots:
    void on_radioButtonWriteToDb_clicked();
    void on_radioButtonWriteToFiles_clicked();
    void on_pushButtonBrowseFolder_clicked();
    void on_pushButtonVtkBrowse_clicked();
    void on_comboBoxShape_currentIndexChanged(int index);
    void on_pushButtonCreate_clicked();
    void on_pushButtonCancel_clicked();


	void slotStart();
    void slotProgress(float percent);
    void slotEnd();
    void slotMsg(std::string msg);
};

#endif // MAINWINDOW_H
