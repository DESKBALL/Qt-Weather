#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

struct dailyWea
{
    QLabel  *day,
        *date,
        *wea,
        *weaImg,
        *airLevel;
};

struct dailyWin
{
    QLabel  *day,
        *win;
    dailyWin(QLabel *day, QLabel *win)
        :day(day), win(win) {}
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void on_miniBtn_clicked();
    void on_clsoeBtn_clicked();
    QString getWeaImage(QString weaImg);

    void readHttpReply(QNetworkReply *reply);
    void updateUI(QByteArray rawData);
    // void updateUI7(QByteArray rawData);
    void drawTempLineHigh();
    void drawTempLineLow();
private:
    Ui::Widget *ui;

    QNetworkAccessManager *manger;

    QString m_url;
    // QString m_7url;

    QPoint m_offset;
    QList<dailyWea> dailyWeaList;
    QList<dailyWin> dailyWinList;

    int maxAve, minAve;
    int maxTemp[6], minTemp[6];

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);
};
#endif // WIDGET_H
