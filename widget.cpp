#include "widget.h"
#include "ui_widget.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(QSize(500, 850));
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->manger = new QNetworkAccessManager(this);

    this->m_url = "http://v0.yiketianqi.com/free/v2031?appid=57852774&appsecret=jiLcT2tk&aqi=1";

    connect(this->manger, SIGNAL(finished(QNetworkReply*)), this, SLOT(readHttpReply(QNetworkReply*)));
    manger->get(QNetworkRequest(QUrl(this->m_url)));


    dailyWeaList.append({ui->day, ui->date, ui->weatherLabel, ui->weatherSmartIcon, ui->air});
    dailyWeaList.append({ui->day_2, ui->date_2, ui->weatherLabel_2, ui->weatherSmartIcon_2, ui->air_2});
    dailyWeaList.append({ui->day_3, ui->date_3, ui->weatherLabel_3, ui->weatherSmartIcon_3, ui->air_3});
    dailyWeaList.append({ui->day_4, ui->date_4, ui->weatherLabel_4, ui->weatherSmartIcon_4, ui->air_4});
    dailyWeaList.append({ui->day_5, ui->date_5, ui->weatherLabel_5, ui->weatherSmartIcon_5, ui->air_5});
    dailyWeaList.append({ui->day_6, ui->date_6, ui->weatherLabel_6, ui->weatherSmartIcon_6, ui->air_6});

    dailyWinList.append(dailyWin(ui->day_7, ui->win_1));
    dailyWinList.append(dailyWin(ui->day_8, ui->win_2));
    dailyWinList.append(dailyWin(ui->day_9, ui->win_3));
    dailyWinList.append(dailyWin(ui->day_10, ui->win_4));
    dailyWinList.append(dailyWin(ui->day_11, ui->win_5));
    dailyWinList.append(dailyWin(ui->day_12, ui->win_6));

    ui->maxTemperature->installEventFilter(this);
    ui->minTemperature->installEventFilter(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString city = ui->lineEdit->text();
    ui->lineEdit->clear();
    if(city.back() == "市")
    {
        QMessageBox::warning(this, "提示", "末尾不需要加市");
        return;
    }
    manger->get(QNetworkRequest(QUrl(this->m_url + "&city=" + city)));
}

void Widget::on_miniBtn_clicked()
{
    this->showMinimized();
}

void Widget::on_clsoeBtn_clicked()
{
    this->close();
}

QString Widget::getWeaImage(QString weaImg)
{
    // xue、lei、shachen、wu、bingbao、yun、yu、yin、qing
    if(weaImg == "xue") return ":/icon/snowy.svg";
    if(weaImg == "lei") return ":/icon/thunder.svg";
    if(weaImg == "shachen") return ":/icon/fly_ash.svg";
    if(weaImg == "wu") return ":/icon/fog.svg";
    if(weaImg == "bingbao") return ":/icon/hail.svg";
    if(weaImg == "yun") return ":/icon/cloudy.svg";
    if(weaImg == "yu") return ":/icon/rainy.svg";
    if(weaImg == "yin") return ":/icon/cloudy.svg";
    if(weaImg == "qing") return ":/icon/clear.svg";
    return ":/icon/model/Error.svg";
}

void Widget::readHttpReply(QNetworkReply *reply)
{
    int resCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(reply->error() != QNetworkReply::NoError || resCode != 200)
    {
        QMessageBox::warning(this, "错误" + QString::number(resCode), reply->errorString());
        return;
    }
    QByteArray rawData = reply->readAll();
    reply->deleteLater();
    updateUI(rawData);
}

void Widget::updateUI(QByteArray rawData)
{
    QJsonDocument jsonObj = QJsonDocument::fromJson(rawData);
    if(jsonObj.isNull() || !jsonObj.isObject())
    {
        QMessageBox::warning(this, "错误", "HTTP返回数据解析错误");
        return;
    }

    QJsonObject data = jsonObj.object();
    if(data.find("errcode") != data.end())
    {
        QMessageBox::warning(this, "error", data["errmsg"].toString());
        return;
    }

    // 城市
    QString city = data["city"].toString();
    ui->city->setText(city + "市");

    // 设置日期
    QString date = data["date"].toString();
    ui->labelToday->setText(date);

    // 获取实况天气
    QJsonObject object = data["day"].toObject();

    // 气温
    QString tem = object["tem"].toString();
    QString tem1 = object["tem1"].toString();
    QString tem2 = object["tem2"].toString();
    ui->temperature->setText(tem + "℃");
    ui->temperatureRange->setText(tem1 + '-' + tem2 + "℃");

    // 天气
    QString wea = object["wea"].toString();
    QString weaImg = object["wea_img"].toString();
    ui->weatherToday->setText(wea);
    ui->weatherIcon->setPixmap(getWeaImage(weaImg));

    // 风向
    QString win = object["win"].toString();
    QString winSpeed = object["win_speed"].toString();
    ui->labelFXType->setText(win);
    ui->labelFXLeave->setText(winSpeed);

    // PM2.5
    QString pm25 = object["air_pm25"].toString();
    ui->labelPM25Leave->setText(pm25);

    // 湿度
    QString humidity = object["humidity"].toString();
    ui->labelHumidityLeave->setText(humidity);

    // 空气质量
    QString airlevel = object["air_level"].toString();
    ui->labelAirIndexLeave->setText(airlevel);

    // 如果有预警则输出
    QJsonArray alarm = object["alarm"].toArray();
    if(!alarm.isEmpty())
    {
        QJsonObject e = alarm[0].toObject();
        QString tip = e["alarm_title"].toString();
        ui->labeltip->setText(tip);
    }


    // 更新6日天气
    maxAve = 0; minAve = 0;
    QJsonArray week = data["week"].toArray();
    for(int i = 0; i < 6; i++)
    {
        QJsonObject obj = week[i].toObject();

        // 日期
        if(i > 1)
        {
            QString day = obj["week"].toString();
            dailyWeaList[i].day->setText(day);
        }
        else if(i == 0)
        {
            QString date = ui->labelToday->text();
            ui->labelToday->setText(date + ' ' + obj["week"].toString());
        }
        QStringList dateList = obj["date"].toString().split('-');
        dailyWeaList[i].date->setText(dateList[1] + "\\" + dateList[2]);

        // 天气
        dailyWeaList[i].wea->setText(obj["wea"].toString());
        dailyWeaList[i].weaImg->setPixmap(getWeaImage(obj["wea_img"].toString()));

        // 空气质量
        QString air_level = obj["air_level"].toString();
        dailyWeaList[i].airLevel->setText(air_level);
        if(air_level == "优")
            dailyWeaList[i].airLevel->setStyleSheet("background-color: green; border-radius: 5px;");
        else if(air_level == "良")
            dailyWeaList[i].airLevel->setStyleSheet("background-color: rgb(150, 213, 32); border-radius: 5px;");
        else
            dailyWeaList[i].airLevel->setStyleSheet("background-color: red; border-radius: 5px;");

        // 风向
        dailyWinList[i].day->setText(obj["win"].toArray()[0].toString());
        dailyWinList[i].win->setText(obj["win_speed"].toString());

        // 计算温度平均值
        maxTemp[i] = obj["tem1"].toString().toInt();
        minTemp[i] = obj["tem2"].toString().toInt();
        maxAve += maxTemp[i];
        minAve += minTemp[i];
    }
    maxAve /= 6;
    minAve /= 6;
    ui->maxTemperature->update();
    ui->minTemperature->update();
}

void Widget::drawTempLineHigh()
{
    QPainter painter(ui->maxTemperature);
    painter.setPen(QPen(Qt::yellow));
    painter.setBrush(Qt::yellow);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QPoint point[6];
    int shift = ui->dailyWeather->width();
    for(int i = 0; i < 6; i++)
    {
        point[i].setX(i * shift + shift / 2);
        point[i].setY(ui->maxTemperature->height() / 2 - (maxTemp[i] - maxAve) * 3);
        // qDebug() << point[i] << dailyWeaList[i].day->x() << i;

        // 画点
        painter.drawEllipse(point[i], 3, 3);
        // 画线
        if(i > 0) painter.drawLine(point[i], point[i-1]);
        // 画温度
        painter.drawText(point[i].x() - 10, point[i].y() - 10, QString::number(maxTemp[i]) + "℃");
    }
}

void Widget::drawTempLineLow()
{
    QPainter painter(ui->minTemperature);
    painter.setPen(QPen(Qt::blue));
    painter.setBrush(Qt::blue);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QPoint point[6];
    int shift = ui->dailyWeather->width();
    for(int i = 0; i < 6; i++)
    {
        point[i].setX(i * shift + shift / 2);
        point[i].setY(ui->minTemperature->height() / 2 - (minTemp[i] - minAve) * 3);
        // qDebug() << point[i] << dailyWeaList[i].day->x() << i;

        // 画点
        painter.drawEllipse(point[i], 3, 3);
        // 画线
        if(i > 0) painter.drawLine(point[i], point[i-1]);
        // 画温度
        painter.drawText(point[i].x() - 10, point[i].y() - 10, QString::number(minTemp[i]) + "℃");
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_offset = event->pos();
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() - m_offset);
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->maxTemperature && event->type() == QEvent::Paint)
    {
        drawTempLineHigh();
        return true;
    }
    if(watched == ui->minTemperature && event->type() == QEvent::Paint)
    {
        drawTempLineLow();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

