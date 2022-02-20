#include "nasdaqsweep.h"
#include "gFi.cpp"
#include <iostream>

//Nasdaq cycler is made a class so it can be created and accessed by other objects 
//Contructor contains the network manager and interval that it updates
NASDAQSweep::NASDAQSweep(QNetworkAccessManager* mgr, int Interv,QWidget *parent)
    : QWidget{parent}
{
    //Label for logging to UI
    logLab = new QLabel(this);
    nMgr = mgr;
    //Creating timer and connecting its timeout to the first step in the cycle
    nasdaqTimer = new QTimer;
    nasdaqTimer->setInterval(Interv);
    connect(nasdaqTimer, &QTimer::timeout, this, &NASDAQSweep::step1);
    nasdaqTimer->start();
}

void NASDAQSweep::step1()
{
    //This step gets all Nasdaq quotes and sends reply to step2
    QString url = "https://financialmodelingprep.com/api/v3/quotes/nasdaq?apikey=xxxx";
    QNetworkReply *nReply = nMgr->get(QNetworkRequest(QUrl(url)));
    connect(nReply, &QNetworkReply::finished, this, &NASDAQSweep::step2);

}

void NASDAQSweep::step2()
{
    // Here we get a pointer to the reply that called the function
    QNetworkReply *nReply = qobject_cast<QNetworkReply*>(sender());
    
    // We now turn it into a QByteArray to read the Json, checking for null ptr and if the Byte Array is empty in the case of an API error. We return if this is the case.
    QByteArray val = gFi::initJsonReply(nReply);
    if(val.isEmpty()){
        return;
    }
    
    // We now prepare our insert with all values going into the table, preventing SQL injection.
    QJsonArray arr = QJsonDocument::fromJson(val).array();
    QSqlQuery q;
    q.prepare("INSERT INTO fmp_nasdaq_quotes  VALUES (:ticker, 	:name, :price, :changePercent, :change, :dayLow, :dayHigh, :yearHigh, :yearLow, :marketCap, :priceAvg50, :priceAvg200	, :volume	, :avgVolume	, :exchange	, :open	, :previousClose	, :eps	, :pe	, :earningsDate	, :sharesOutstanding	, :timestamp )");
    QVariantList ticker;
    QVariantList name;
    QVariantList price;
    QVariantList changePercent;
    QVariantList change;
    QVariantList dayLow;
    QVariantList dayHigh;
    QVariantList yearHigh;
    QVariantList yearLow;
    QVariantList marketCap;
    QVariantList priceAvg50;
    QVariantList priceAvg200;
    QVariantList volume;
    QVariantList avgVolume;
    QVariantList exchange;
    QVariantList open;
    QVariantList previousClose;
    QVariantList eps;
    QVariantList pe;
    QVariantList earningsDate;
    QVariantList sharesOutstanding;
    QVariantList timestamp;
    
    // Each json object in the array contains the values we need so we append the QVariantLists which we will use to bind the values to the prepared statement.
    for(auto m: arr){
        QJsonObject obj = m.toObject();
        ticker.append(obj["symbol"].toString());
        name.append(obj["name"].toString());
        price.append(obj["price"].toDouble());
        changePercent.append(obj["changesPercentage"].toDouble());
        change.append(obj["change"].toDouble());
        dayLow.append(obj["dayLow"].toDouble());
        dayHigh.append(obj["dayHigh"].toDouble());
        yearHigh.append(obj["yearHigh"].toDouble());
        yearLow.append(obj["yearLow"].toDouble());
        marketCap.append(obj["marketCap"].toDouble());
        priceAvg50.append(obj["priceAvg50"].toDouble());
        priceAvg200.append(obj["priceAvg200"].toDouble());
        volume.append(obj["volume"].toDouble());
        avgVolume.append(obj["avgVolume"].toDouble());
        exchange.append(obj["exchange"].toString());
        open.append(obj["open"].toDouble());
        previousClose.append(obj["previousClose"].toDouble());
        eps.append(obj["eps"].toDouble());
        pe.append(obj["pe"].toDouble());
        if(obj["earningsAnnouncement"].toString() == ""){
            earningsDate.append(QVariant(QVariant::String));
        }
        else{
        earningsDate.append(obj["earningsAnnouncement"].toString());
        }
        sharesOutstanding.append(obj["sharesOutstanding"].toDouble());
        timestamp.append("NOW()");
    }
    q.addBindValue(ticker);
    q.addBindValue(name);
    q.addBindValue(price);
    q.addBindValue(changePercent);
    q.addBindValue(change);
    q.addBindValue(dayLow);
    q.addBindValue(dayHigh);
    q.addBindValue(yearHigh);
    q.addBindValue(yearLow);
    q.addBindValue(marketCap);
    q.addBindValue(priceAvg50);
    q.addBindValue(priceAvg200);
    q.addBindValue(volume);
    q.addBindValue(avgVolume);
    q.addBindValue(exchange);
    q.addBindValue(open);
    q.addBindValue(previousClose);
    q.addBindValue(eps);
    q.addBindValue(pe);
    q.addBindValue(earningsDate);=
    q.addBindValue(sharesOutstanding);
    q.addBindValue(timestamp);
    
    //Show error if execution fails
    if(!q.execBatch()){
        logLab->setText(q.lastError().text());
        std::cout<<q.lastError().text().toStdString()<<std::endl;
    }

    //We now search for new tickers that might have been added to the Nasdaq in this cycle
    //If there are new tickers we need to add them to our ticker list
    QVariantList missingTicks;
    QVariantList missingNames;
    QVariantList now;
    QVariantList exch;
    QSqlQuery find;
    find.exec("SELECT DISTINCT ticker FROM portfolio_table");
    QVector<QString> existticks;
    if(find.isSelect()){
        int size = find.record().count();
        while(find.next()){
                for(int k = 0;k< size; k++){
                    existticks.append(find.value(k).toString());
            }
        }
    }
    for(int k = 0; k<ticker.size(); k++){
        bool found = false;
        for(int i=0; i<existticks.size(); i++){
        if(ticker[k].toString() ==  existticks[i] ){
            found = true;
        }
        if ( i == existticks.size() - 1 && found == false) {
            missingTicks.append(ticker[k]);
            missingNames.append(name[k]);
            now.append("NOW()");
            exch.append("NASDAQ");
        }
    }

}
    // We have new tickers
    if(missingTicks.size() > 1){
        std::cout<<"new tickers"<<std::endl;
            QSqlQuery qN;
            qN.prepare("INSERT INTO porfolio_table VALUES (:updated, :ticker, :name, :exchange) ");
            qN.addBindValue(now);
            qN.addBindValue(missingTicks);
            qN.addBindValue(missingNames);
            qN.addBindValue(exch);
            if(!qN.execBatch()){
                std::cout<<q.lastError().text().toStdString()<<std::endl;

            };

        }

    // Update the table
    for(int i=0; i<ticker.size(); i++){
    q.prepare(""
              "UPDATE portfolio_table SET updated = ?,  price = ?, change = ?, changePercent = ?, volume = ?, dayLow = ?, dayHigh = ?, open = ?, previousClose = ?, pe = ?, eps = ? WHERE ticker = '" + ticker[i].toString() + "'"
              "");
    q.addBindValue(timestamp[i]);
    q.addBindValue(price[i]);
    q.addBindValue(change[i]);
    q.addBindValue(changePercent[i]);
    q.addBindValue(volume[i]);
    q.addBindValue(dayLow[i]);
    q.addBindValue(dayHigh[i]);
    q.addBindValue(open[i]);
    q.addBindValue(previousClose[i]);
    q.addBindValue(pe[i]);
    q.addBindValue(eps[i]);
    if(!q.exec()){
        std::cout<<q.lastError().text().toStdString()<<std::endl;
    };
    }


}
