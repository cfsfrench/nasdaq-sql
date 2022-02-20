#ifndef NASDAQSWEEP_H
#define NASDAQSWEEP_H

#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtSql>


class NASDAQSweep : public QWidget
{
    Q_OBJECT
public:
    explicit NASDAQSweep(QNetworkAccessManager*,int Interv,QWidget *parent = nullptr);
private:
    QTimer *nasdaqTimer;
    int nasdaqCount = 0;
    QNetworkAccessManager *nMgr;
    void step1();
    void step2();
    QLabel *logLab;

signals:

};

#endif // NASDAQSWEEP_H
