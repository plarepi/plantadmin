#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QSqlTableModel>
#include <QtCharts>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_button_db_create_clicked();

    void on_pushButton_clicked();

    void on_dbtable_nl_Button_clicked();

    void on_dbtableView_clicked(const QModelIndex &index);

    void on_loadentry_Button_clicked();

    void on_createdt_Button_clicked();

    void on_entryView_clicked(const QModelIndex &index);

    void on_rawdatanl_Button_clicked();

    void on_rawdatanl_Button_2_clicked();

    void on_rawDataTable_clicked(const QModelIndex &index);

    void Render_Graph();

    void PlotData(QVector<QVector<float>> *x_values, QVector<QVector<QDateTime>> *y_values, QVector<QDateTime> *sowing_dates, QVector<QString> *entry_names);

    void on_reload_Graph_clicked();

    void readmodelFiles();

    void on_graph_vis_comboBox_2_currentIndexChanged(const QString &arg1);

    float Model_Function(float x);

    void setDefaultDate();

    void on_rawdataDel_Button_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase current_DB;
    QString current_entry;
    QModelIndex current_entry_Index;
    QSqlTableModel currentmonitorTable;
    int current_raw_entry;
    QModelIndex last_EntryIndex;

    QChartView *chartView = new QChartView;
    QDateTimeAxis *axisX = new QDateTimeAxis;
    QValueAxis *axisY = new QValueAxis;
    QValueAxis *axisY2 = new QValueAxis;


    QString model_type;
    QString model_units;
    QVector <float> model_params {0, 0, 0, 0};
    void readDbFiles();
    void loadDb();

};
#endif // MAINWINDOW_H
