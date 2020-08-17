#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QSqlTableModel>
#include <QDateTime>
#include <QHeaderView>
#include <QClipboard>

#include <QtCharts>
#include <QScatterSeries>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readDbFiles();
    readmodelFiles();
    setDefaultDate();
    ui->verticalLayout_7->addWidget(chartView);


}

MainWindow::~MainWindow()
{
    current_DB.commit();
    current_DB.close();
    delete ui;
}
void MainWindow::readDbFiles(){
    ui->db_ComboBox->clear();
    QDir path = QDir::currentPath();
    QStringList filters;
    filters << "*.db";
    path.setNameFilters(filters);
     QStringList files = path.entryList(QDir::Files);
    ui->db_ComboBox->addItems(files);

}

void MainWindow::readmodelFiles(){
    ui->graph_vis_comboBox_2->clear();
    QDir path = QDir::currentPath()+"/modelos/";
    QStringList filters;
    filters << "*.txt";
    path.setNameFilters(filters);
     QStringList files = path.entryList(QDir::Files);
    ui->graph_vis_comboBox_2->addItems(files);

}

void MainWindow::on_button_db_create_clicked()
{
    QDir databasePath;
    QString path = databasePath.currentPath() +"/" + ui->Db_name_input->text() + ".db";
    //qWarning(path.toLatin1());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();

    QSqlQuery query(db);
    query.exec("create table table_ids"
              "(id integer primary key, "
              "nombre varchar(20), "
              "fecha_siembra datetime)");
    db.commit();
    db.close();
    //QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    readDbFiles();
}

void MainWindow::loadDb()
{
    //ClearDb();
    QDir databasePath;
    QString path = databasePath.currentPath() +"/" + ui->db_ComboBox->currentText();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();
    QSqlTableModel *model = new QSqlTableModel(nullptr, db);
    model->setTable("table_ids");
    model->select();

    ui->dbtableView->setModel(model);
    ui->dbtableView->hideColumn(0);
    current_DB = db;
    Render_Graph();

    ui->rawdatanl_Button->setEnabled(false);
    ui->rawdatanl_Button_2->setEnabled(false);
    ui->rawdataDel_Button->setEnabled(false);
}



void MainWindow::on_pushButton_clicked()
{
    loadDb();
    ui->dbtable_nl_Button->setEnabled(true);
    ui->loadentry_Button->setEnabled(false);
    ui->reload_Graph->setEnabled(true);
}

void MainWindow::on_dbtable_nl_Button_clicked()
{
    int lastRow =ui->dbtableView->model()->rowCount();
    QString NewName = ui->nl_textname->toPlainText();
    QDateTime NewDate = ui->nl_dateTime->dateTime();
    ui->dbtableView->model()->insertRow(lastRow);
    ui->dbtableView->model()->setData(ui->dbtableView->model()->index(lastRow, 0), lastRow);
    ui->dbtableView->model()->setData(ui->dbtableView->model()->index(lastRow, 1), NewName);
    ui->dbtableView->model()->setData(ui->dbtableView->model()->index(lastRow, 2), NewDate);
    ui->dbtableView->model()->submit();

    QSqlQuery query(current_DB);
    QString value = QString::number(lastRow);
    QString sql = "create table monitoreo_"+value+" (id integer primary key, dias_siembra float, fecha datetime)";
    query.exec(sql);
    current_DB.commit();

}

void MainWindow::on_dbtableView_clicked(const QModelIndex &index)
{

    int row = index.row();
    QString s = QString::number(row);
    //qWarning(s.toLatin1());
    ui->loadentry_Button->setEnabled(true);
}

void MainWindow::on_loadentry_Button_clicked()
{
    ui->loadentry_Button->setEnabled(false);
    QModelIndexList selection = ui->dbtableView->selectionModel()->selectedIndexes();
    int row = selection.at(0).row();
    QModelIndex index_db = ui->dbtableView->model()->index(row, 0);
    current_entry_Index = index_db;
    QString table_id = ui->dbtableView->model()->data(index_db).toString();
    current_entry = table_id;

    QSqlTableModel *model2 = new QSqlTableModel(nullptr, current_DB);
    model2->setTable("monitoreo_"+current_entry);
    model2->select();
    ui->entryView->setModel(model2);
    ui->entryView->hideColumn(0);

    ui->rawdatanl_Button->setEnabled(false);
    ui->rawdatanl_Button_2->setEnabled(false);
    ui->rawdataDel_Button->setEnabled(false);

}

void MainWindow::on_createdt_Button_clicked()
{

    int lastRow = ui->entryView->model()->rowCount();
    QDateTime NewDate = ui->dt_dateTime->dateTime();
    QModelIndex SowDate_index = ui->dbtableView->model()->index(current_entry_Index.row(), 2);
    QDateTime SowDate = ui->dbtableView->model()->data(SowDate_index).toDateTime();
    float Days_passed = SowDate.secsTo(NewDate)/(3600*24);

    ui->entryView->model()->insertRow(lastRow);
    ui->entryView->model()->setData(ui->entryView->model()->index(lastRow, 0), lastRow);
    ui->entryView->model()->setData(ui->entryView->model()->index(lastRow, 1), Days_passed);
    ui->entryView->model()->setData(ui->entryView->model()->index(lastRow, 2), NewDate);
    ui->entryView->model()->submit();
    //qWarning(QString::number(ui->entryView->model()->rowCount()).toLatin1());


    QSqlQuery query(current_DB);
    QString value = QString::number(lastRow);
    QString sql = "create table monitoreo_"+current_entry+"data"+value+" (id integer primary key, Cx float, Cy float,	Cz	float, width float, length float, height float,	t_height float, R	integer, G	integer, B	integer, area float, volume float, param1 float, param2 float)";
    query.exec(sql);
    current_DB.commit();

}

void MainWindow::on_entryView_clicked(const QModelIndex &index)
{
    last_EntryIndex = index;
    int row = index.row();
    current_raw_entry = row;
    QModelIndex index_id = ui->entryView->model()->index(row, 0);
    QString index_data =  ui->entryView->model()->data(index_id).toString();


    QSqlTableModel *model3 = new QSqlTableModel(nullptr, current_DB);
    model3->setTable("monitoreo_"+current_entry+"data"+index_data);
    model3->setEditStrategy(QSqlTableModel::OnFieldChange);
    model3->select();
    ui->rawDataTable->setModel(model3);
    ui->rawDataTable->hideColumn(0);
    ui->rawdatanl_Button->setEnabled(true);
    ui->rawdataDel_Button->setEnabled(true);
    ui->rawdatanl_Button_2->setEnabled(false);
}

void MainWindow::on_rawdatanl_Button_clicked()
{
    int lastRow = ui->rawDataTable->model()->rowCount();
    ui->rawDataTable->model()->insertRow(lastRow);
    ui->rawDataTable->model()->setData(ui->rawDataTable->model()->index(lastRow, 0), lastRow);
    ui->rawDataTable->model()->submit();
}


void MainWindow::on_rawdatanl_Button_2_clicked()
{
    ui->rawdatanl_Button_2->setEnabled(false);
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString originalText = clipboard->text();
    QStringList rowlist;
    rowlist = originalText.split("\n");

    QModelIndexList selection = ui->rawDataTable->selectionModel()->selectedIndexes();
    int first_column = selection.first().column();
    int first_row = selection.first().row();
    int current_column = first_column;
    int current_row = first_row;

    for (int i = 0; i < rowlist.length(); i++) {
        QStringList current_list = rowlist[i].split("\t");
        //avoid index out of range

        if(current_row >  ui->rawDataTable->model()->rowCount()){
            continue;
        }


        for (int j = 0; j < current_list.length(); j++) {
            //avoid index out of range
            float value = current_list[j].toFloat();
            QModelIndex current_index = ui->rawDataTable->model()->index(current_row, current_column);
            ui->rawDataTable->model()->setData(current_index, value);

            current_column++;

            if(current_column >  ui->rawDataTable->model()->columnCount()){
                j = current_list.length();
            }
            ui->rawDataTable->model()->submit();

        }

        current_column=first_column;
        current_row++;
    }



}

void MainWindow::on_rawDataTable_clicked(const QModelIndex &index)
{

    ui->rawdatanl_Button_2->setEnabled(true);
}


void MainWindow::Render_Graph()
{
    //clear previous data
    chartView->chart()->removeAllSeries();
    chartView->chart()->removeAxis(axisX);
    chartView->chart()->removeAxis(axisY);
    chartView->chart()->removeAxis(axisY2);

    //add axis
    axisX = new QDateTimeAxis;
    axisX->setFormat("dd-MM-yyyy");
    axisX->setTitleText("Fecha");
    chartView->chart()->addAxis(axisX, Qt::AlignBottom);

    axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText(ui->graph_param_comboBox->currentText());
    chartView->chart()->addAxis(axisY, Qt::AlignLeft);

    //add optional axis

    axisY2 = new QValueAxis;
    axisY2->setLabelFormat("%i");
    axisY2->setTitleText(model_units);
    chartView->chart()->addAxis(axisY2, Qt::AlignRight);


    //obtain all values fro database as vectors
    QVector<QVector<float>> x_values;
    QVector<QVector<QDateTime>> y_values;
    QVector<QDateTime> sowing_dates;
    QVector<QString> entry_names;
    PlotData(&x_values, &y_values, &sowing_dates, &entry_names);
    float min_y = 0, max_y = 0;
    QDateTime min_x, max_x;


    //find min date
    for (int i = 0;i <sowing_dates.size()-1; i++) {
        if(i == 0){min_x =sowing_dates[i];}

        int difference = min_x.secsTo(sowing_dates[i+1]);
        if(difference < 0){
             min_x =sowing_dates[i+1];

            }
    }

    for (int i = 0;i <x_values.size(); i++){
        QScatterSeries  *series = new QScatterSeries ;
        QScatterSeries  *model_series = new QScatterSeries;
        QLineSeries *series_avg = new QLineSeries;
        QLineSeries *model_avg = new QLineSeries;
        float avg_sum = 0, sum = 1;

        for (int j = 0; j < x_values[i].size(); j++) {
            //add values to scatter series
            //qWarning(QString::number(x_values[i][j]).toLatin1());
            series->append(y_values[i][j].toMSecsSinceEpoch(), x_values[i][j]);

            //add first value to average
            if(j==0){avg_sum += x_values[i][j];}
            //add values with same date to get averages
            if(j > 0){
                if(y_values[i][j] == y_values[i][j-1])
                {

                    avg_sum += x_values[i][j];
                    sum++;
                }
                else{
                    //add average values
                    series_avg->append(y_values[i][j-1].toMSecsSinceEpoch(), avg_sum/sum);
                    if(!model_type.toStdString().empty()){
                    model_avg->append(y_values[i][j-1].toMSecsSinceEpoch(), Model_Function(avg_sum/sum));
                    }
                    avg_sum = x_values[i][j], sum = 1;

                }
                //this is necesary or last place is not included
                if(j ==  (x_values[i].size()-1))
                {
                    //avg_sum += x_values[i][j];
                    //sum = 1;
                    series_avg->append(y_values[i][j].toMSecsSinceEpoch(), avg_sum/sum);
                    if(!model_type.toStdString().empty()){
                    model_avg->append(y_values[i][j].toMSecsSinceEpoch(), Model_Function(avg_sum/sum));
                    }
                    //qWarning(QString::number(avg_sum/sum).toLatin1());
                }
            }




            //initialize values for averages, mins and maxs to later do comparisons
            if((j == 0) &&(i == 0 )){max_x = y_values[i][j]; max_y = x_values[i][j]; min_y = x_values[i][j]; avg_sum = x_values[i][j];}

            //find min and max values
            if(j < x_values[i].size()-1){
                int difference = max_x.secsTo(y_values[i][j+1]);
                if(difference > 100){
                     max_x = y_values[i][j+1];
                }
                if(max_y < x_values[i][j+1]){max_y = x_values[i][j+1];}
                if(min_y > x_values[i][j+1]){min_y = x_values[i][j+1];}
            }

            //add model values if the proper model was selected
            if(!model_type.toStdString().empty()){
                float value = Model_Function(x_values[i][j]);
                model_series->append(y_values[i][j].toMSecsSinceEpoch(), value);
            }



        }//end of for lopp for the same entry



        //attach axis to series
        if(x_values[i].size()>0){

        //set colors to be consistent
        QColor black(0, 0, 0);
        QColor color(rand() % 255, rand() % 255, rand() % 255);
        QPen pen(black);
        series->setMarkerSize(7);
        series->setColor(color);
        series->setBorderColor(black);
        series_avg->setColor(color);

        model_series->setMarkerSize(7);
        model_series->setColor(color);
        model_series->setBorderColor(black);
        model_series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);

        QPen pen2(Qt::DashLine);
        model_avg->setPen(pen2);
        model_avg->setColor(color);



        //add axis to leyend
        chartView->chart()->addSeries(series);
        chartView->chart()->addSeries(series_avg);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        series_avg->attachAxis(axisX);
        series_avg->attachAxis(axisY);

        if(!model_type.toStdString().empty()){
            chartView->chart()->addSeries(model_series);
            chartView->chart()->addSeries(model_avg);
            model_series->attachAxis(axisX);
            model_series->attachAxis(axisY2);
            model_avg->attachAxis(axisX);
            model_avg->attachAxis(axisY2);
        }


        }

    }
    //end of loop for all entries
    if((ui->graph_vis_comboBox->currentIndex()==1)){
        min_x = ui->Graph_dateTimeEdit->dateTime();
        max_x = ui->Graph_dateTimeEdit_2->dateTime();

    }

    //change range if user defined it to

    axisX->setRange(min_x.addDays(-1), max_x);
    axisY->setRange((min_y-min_y*0.5), (max_y+max_y*0.25));
    chartView->chart()->legend()->disconnect();

    //qWarning(QString::number(chartView->chart()->legend()->markers().size()).toLatin1());
    //delete legends that we dont want
    for (int i = 0;i < chartView->chart()->legend()->markers().size(); i++) {
        if(model_type.toStdString().empty()){
            if(!(i % 2))
            {
                chartView->chart()->legend()->markers()[i]->setVisible(false);
            }
            else{
                if((ui->graph_vis_comboBox->currentIndex()==1) && ((sowing_dates[i/2].daysTo(min_x) < 0) || (sowing_dates[i/2].daysTo(max_x) > 0))){
                    chartView->chart()->legend()->markers()[i]->setVisible(false);
                }else{
                    if(!ui->useName_Radio->isChecked()){chartView->chart()->legend()->markers()[i]->setLabel(sowing_dates[i/2].date().toString());}
                    else{chartView->chart()->legend()->markers()[i]->setLabel(entry_names[i/2]);}

                }
            }
        }else{
            //THIS IS WRONG BUT ONLY WORKS LIKE THIS!!!
            if((i % 4)){
                chartView->chart()->legend()->markers()[i]->setVisible(false);
            }else{
                if((ui->graph_vis_comboBox->currentIndex()==1) && ((sowing_dates[i/4].daysTo(min_x) < 0) || (sowing_dates[i/4].daysTo(max_x) > 0))){
                    chartView->chart()->legend()->markers()[i]->setVisible(false);
                }else{
                    if(!ui->useName_Radio->isChecked()){chartView->chart()->legend()->markers()[i]->setLabel(sowing_dates[i/4].date().toString());}
                    else{chartView->chart()->legend()->markers()[i]->setLabel(entry_names[i/4]);}
                }
            }

        }
    }


}


void MainWindow::PlotData(QVector<QVector<float>> *x_values, QVector<QVector<QDateTime>> *y_values, QVector<QDateTime> *sowing_dates, QVector<QString> *entry_names)
{

    //cycle all entries
    int num_entries = ui->dbtableView->model()->rowCount();
    for (int i = 0;i < num_entries; i++) {
        //load entry table (first)
        QModelIndex current_entry_Index = ui->dbtableView->model()->index(i, 0);
        QModelIndex current_name_Index = ui->dbtableView->model()->index(i, 1);
        QModelIndex current_date_Index = ui->dbtableView->model()->index(i, 2);
        QString current_entry = ui->dbtableView->model()->data(current_entry_Index).toString();
        entry_names->append( ui->dbtableView->model()->data(current_name_Index).toString());
        sowing_dates->append(ui->dbtableView->model()->data(current_date_Index).toDateTime());
        //load second table (measurements per entry)
        QSqlQueryModel *temp_entrie_model = new QSqlQueryModel;

        temp_entrie_model->setQuery("SELECT id, dias_siembra, fecha FROM monitoreo_"+current_entry);
        QVector<float> temp_x_values;
        QVector<QDateTime> temp_y_values;

        for (int j = 0;j < temp_entrie_model->rowCount(); j++) {
            //load tables with raw data from user

            QDateTime current_datetime  = temp_entrie_model->data(temp_entrie_model->index(j, 2)).toDateTime();
            QString current_rawData = temp_entrie_model->data(temp_entrie_model->index(j, 0)).toString();
            QSqlQuery query(current_DB);
            query.exec("SELECT "+ ui->graph_param_comboBox->currentText() + " FROM monitoreo_"+current_entry+"data"+current_rawData);
            while (query.next()) {
                QString name = query.value(0).toString();
                //qWarning(name.toLatin1());
                if(!query.value(0).toString().isNull()){
                    float current_data = query.value(0).toFloat();
                    temp_x_values.append(current_data);
                    temp_y_values.append(current_datetime);
                }

            }
            /*
            QSqlQueryModel *temp_rawData_model = new QSqlQueryModel;
            //temp_x_values[];
            temp_rawData_model->setQuery("SELECT "+ ui->graph_param_comboBox->currentText() + " FROM monitoreo_"+current_entry+"data"+current_rawData);
            qWarning("number of rows "+QString::number(temp_rawData_model->rowCount()).toLatin1());
            for (int k = 0;k < temp_rawData_model->rowCount(); k++) {
                QString data = temp_rawData_model->data(temp_entrie_model->index(k, 0)).toString();
                qWarning(data.toLatin1());
                if(temp_rawData_model->data(temp_entrie_model->index(k, 0)).toString() == ""){
                    //qWarning("EMPTY DATA");
                }
                else{
                float current_data = temp_rawData_model->data(temp_entrie_model->index(k, 0)).toFloat();
                temp_x_values.append(current_data);
                temp_y_values.append(current_datetime);
                }
            }
            */

        }
        x_values->append(temp_x_values);
        y_values->append(temp_y_values);

    }

}

void MainWindow::on_reload_Graph_clicked()
{
    Render_Graph();
}



void MainWindow::on_graph_vis_comboBox_2_currentIndexChanged(const QString &arg1)
{

    QString path = QDir::currentPath()+"/modelos/"+ui->graph_vis_comboBox_2->currentText();

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);
    model_type = "";
    int i = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        if(i == 0){
            ui->model_description->setText(line);
        }
        if(i == 1){
            model_units = line;
        }
        if(i == 2){
            model_type = line;
        }
        if((i>2) && (i <= 5)){
            model_params[i-3] = line.toFloat();
        }
        i++;
    }

    file.close();
}

float MainWindow::Model_Function(float x){
    if(model_type == "EXPONENTIAL"){
        return model_params[0]*exp(model_params[1]*x);
    }
    if(model_type == "LOGARITMIC"){
        return model_params[0]*log(model_params[1]*x);
    }
    if(model_type == "LINEAR"){
        return model_params[0]*x+model_params[1];
    }
    if(model_type == "POLYNOMIAL2"){
        return model_params[0]*x*x+model_params[1]*x+model_params[2];
    }
return 0;
}


void MainWindow::setDefaultDate(){
    ui->nl_dateTime->setDateTime(QDateTime::currentDateTime());
    ui->dt_dateTime->setDateTime(QDateTime::currentDateTime());

}

void MainWindow::on_rawdataDel_Button_clicked()
{
    int lastrow = ui->rawDataTable->model()->rowCount()-1;
    QSqlQuery query(current_DB);
    ui->rawDataTable->model()->removeRow(lastrow);
    ui->rawDataTable->model()->submit();
    ui->rawDataTable->update();
    current_DB.commit();
    //qt is garbage, so this must be invoked again to refresh
    on_entryView_clicked(last_EntryIndex);
}
