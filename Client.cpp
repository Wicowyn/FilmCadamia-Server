#include "Client.h"
#include "DataType.h"

Client::Client(QTcpSocket *sock, QMap<QString, GRAmis*>& lol, QObject *parent):
    QObject(parent), GPAmis(lol){
    socket=sock;
    connecter=false;
    connect(socket, SIGNAL(readyRead()), this, SLOT(newData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

QString Client::name(){
    return login;
}

void Client::newData(){
    static qint64 tailleMs=0;
    QDataStream in(socket);
    if(tailleMs==0){
        if(socket->bytesAvailable()<(int)sizeof(qint64)) return;
        in>>tailleMs;
    }
    if(socket->bytesAvailable()<tailleMs) return;
    qint16 command;
    in>>command;

    QString name, mdp;
    QVector<int> data;
    QMap<QString, int> map;
    qDebug()<<login<<"reception de donnée de type: "<<command;

    if(!connecter){
        switch(command){
        case Fcn::logIn:
            in>>login;
            in>>password;
            trialConnect();
            break;
        case Fcn::requestCreateAccount:
            UserInfo info;
            in>>info;
            createAccount(info);
            break;
        }
    }
    else{
        switch(command){
        case Fcn::requestNewGP:
            in>>name;
            in>>mdp;
            createGP(name, mdp);
            break;
        case Fcn::requestJoinGP:
            in>>name;
            in>>mdp;
            joinGP(name, mdp);
            break;
        /*case Fcn::requestAllInfoFilmOf:
            in>>name;
            sendAllInfoFilmOf(name);
            break;
        /*case Fcn::newLibrary:
            in>>library;
            break;*/
        case Fcn::updateLibrary:
            in>>data;
            updateLibrary(data);
            break;
        case Fcn::checkFriend:
            in>>map;
            checkFriend(map);
            break;
        case Fcn::requestEtatAllGP:
            sendAllGroup();
            break;
        }
    }
    tailleMs=0;
}

void Client::checkFriend(QMap<QString, int> map){
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);
    QList<MAJUser> list;
    data<<qint64(0);
    data<<qint16(Fcn::updateFriendLibrary);
    for(QMap<QString, int>::iterator it=map.begin(); it!=map.end(); it++){
        int vs=Client::getVersionOf(it.key());
        if(it.value()!=vs) list.append(MAJUser(it.key(), vs, Client::getLibraryOf(it.key())));
    }
    data<<list;
    data.device()->seek(0);
    data<<qint64(paquet.size()-sizeof(qint64));
    socket->write(paquet);
}

void Client::createAccount(UserInfo info){
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);
    data<<qint64(0);
    if(Client::createMembre(info)) data<<qint16(Fcn::yesCreateAccount);
    else data<<qint16(Fcn::notCreateAccount);
    data.device()->seek(0);
    data<<qint64(paquet.size()-sizeof(qint64));
    socket->write(paquet);
}

void Client::updateLibrary(QVector<int> data){
   QList<int> library=Client::getLibraryOf(login);
    for(int i=0; i<data.count(); i++){
        if(data[i]<0) library.removeAll(-1*data[i]);
        else if(!library.contains(data[i])) library.push_back(data[i]);
    }
    QByteArray paquet;
    QDataStream in(&paquet, QIODevice::WriteOnly);
    in<<library;
    QSqlQuery query;
    query.exec("UPDATE Membre SET films='"+QString(paquet.data())+"' WHERE login='"+login+"'");
    query.exec("SELECT version FROM membre WHERE login='"+login+"'");
    query.next();
    int version=query.value(0).toInt()+1;
    query.exec("UPDATE Membre SET version="+QString::number(version)+" WHERE login='"+login+"'");
}

/*QMap<QString, QString> Client::allInfoFilm(){
    return library;
}
/*
void Client::sendAllInfoFilmOf(QString name){
    QMap<QString, GRAmis*>::iterator it=myGroup.begin();
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);
    data<<qint64(0);
    data<<qint16(Fcn::allInfoFilmOf);
    data<<name;
    for(QMap<QString, GRAmis*>::iterator it=myGroup.begin() ; it!=myGroup.end() && paquet.isNull(); it++){
        if(it.value()->contains(name)){
            data<<it.value()->allInfoFilmOf(name);
        }
    }
    data.device()->seek(0);
    data<<qint64(paquet.size()-sizeof(qint64));
    qDebug()<<login<<"Envois toute les infosFilm de"<<name;
    socket->write(paquet);
}
//*/
void Client::joinGP(QString name, QString mdp){
    qDebug()<<login<<"demande d'accés au groupe: "<<name<<"/"<<mdp;
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);
    data<<qint64(sizeof(qint16));
    if(GPAmis.contains(name)){
        if(GPAmis[name]->join(this, mdp)){
            myGroup[name]=GPAmis[name];
            data<<qint16(Fcn::yesJoinGP);
            saveListGP();
            qDebug()<<login<<"accés autorisé";
        }
        else{
            data<<qint16(Fcn::notJoinGP);
            data<<QString("Accés refusé");
            qDebug()<<login<<"accés refusé";
        }
    }
    else{
        if(GRAmis::exists(name)){
            GPAmis[name]=new GRAmis(name, GPAmis);
            if(GPAmis[name]->join(this, mdp)){
                myGroup[name]=GPAmis[name];
                data<<qint16(Fcn::yesJoinGP);
                saveListGP();
                qDebug()<<login<<"accés autorisé";
            }
            else{
                delete GPAmis[name];
                GPAmis.remove(name);
                data<<qint16(Fcn::notJoinGP);
                data<<QString("Accés refusé");
                qDebug()<<login<<"accés refusé";
            }
        }
        else{
            data<<qint16(Fcn::notJoinGP);
            data<<QString("Groupe innexistant");
            qDebug()<<login<<"groupe innexistant";
        }
    }
    socket->write(paquet);
}

void Client::createGP(QString name, QString mdp){
    qDebug()<<login<<"demande de création du groupe: "<<name<<"/"<<mdp;
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);
    data<<qint64(sizeof(qint16));

    if(GRAmis::exists(name)){
        data<<qint16(Fcn::notNewGP);
        qDebug()<<login<<"echec";
    }
    else{
        qDebug()<<login<<"réussite";
        GRAmis::createGP(name, mdp);
        data<<qint16(Fcn::yesNewGP);
    }
    socket->write(paquet);
}

void Client::trialConnect(){
    qDebug()<<login<<"Tentative de connection avec: "<<login<<"/"<<password;
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);
    QSqlQuery query;
    query.exec("SELECT password FROM Membre WHERE login='"+login+"'");
    query.next();

    if(query.value(0).toString()==password){
        connecter=true;
        qDebug()<<login<<"Connection autorisée";

        QStringList list;
        query.exec("SELECT groupes FROM membre WHERE login ='"+login+"'");
        query.next();
        list=query.value(0).toString().split("#");

        for(int i=0; i<list.count(); i++){
            if(GPAmis.contains(list[i])){
                myGroup[list[i]]=GPAmis[list[i]];
                qDebug()<<"Groupe"<<list[i]<<"existe et co";
            }
            else{
                if(GRAmis::exists(list[i])){
                    GPAmis[list[i]]=new GRAmis(list[i], GPAmis);
                    myGroup[list[i]]=GPAmis[list[i]];
                    qDebug()<<"Groupe"<<list[i]<<"existe et rajout";
                }
                else{
                    qDebug()<<"Groupe"<<list[i]<<"inexistant";
                }           //içi informer le client de l'inexistence du groupe
            }               //et proposer de le créer
        }
        for(QMap<QString, GRAmis*>::iterator it=myGroup.begin(); it!=myGroup.end(); it++) it.value()->connect(this);
        out<<qint64(sizeof(qint16));
        out<<qint16(Fcn::logInOn);
        socket->write(paquet);
        emit connected();
    }
    else{
        connecter=false;
        out<<qint64(sizeof(qint16));
        out<<qint16(Fcn::logInOff);
        socket->write(paquet);
        qDebug()<<"Connection refusée";
    }
}

void Client::sendAllGroup(){
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);

    data<<qint64(0);
    data<<qint16(Fcn::etatAllGP);
    data<<myGroup.count();
    for(QMap<QString, GRAmis*>::iterator it=myGroup.begin(); it!=myGroup.end(); it++){
        data<<it.key();
        data<<it.value()->getList();
    }
    data.device()->seek(0);
    data<<qint64(paquet.size()-sizeof(qint64));
    qDebug()<<login<<"Envois de l'état des groupes";
    socket->write(paquet);
}

void Client::sendUser(QString name, bool etat){
    QByteArray paquet;
    QDataStream data(&paquet, QIODevice::WriteOnly);

    data<<qint64(0);
    data<<qint16(Fcn::etatUser);
    data<<name;
    data<<etat;
    data.device()->seek(0);
    data<<qint64(paquet.size()-sizeof(qint64));

    qDebug()<<login<<"Envois de l'état de"<<name<<etat;
    socket->write(paquet);
}

bool Client::saveListGP(){
    QStringList list;
    for(QMap<QString, GRAmis*>::iterator it=GPAmis.begin(); it!=GPAmis.end(); it++) list+=it.key();
    QSqlQuery query;
    return query.exec("UPDATE Membre SET groupes='"+list.join("#")+"' WHERE login='"+login+"'");
}

bool Client::createMembre(UserInfo info){
    QSqlQuery query;
    return query.exec("INSERT INTO Membre (login, password, nom, prenom, email) VALUES ('"+info.login+"', '"+info.password+"', '"+info.firstName+"', '"+info.secondName+"', '"+info.eMail+"')");
}

bool Client::exists(QString name){
    QSqlQuery query;
    query.exec("SELECT login FROM Membre WHERE login='"+name+"'");
    return query.next();
}

QList<int> Client::getLibraryOf(QString name){
    QSqlQuery query;
    query.exec("SELECT films FROM membre WHERE login='"+name+"'");
    query.next();
    QByteArray paquet=query.value(0).toByteArray();
    QDataStream data(&paquet, QIODevice::ReadOnly);
    QList<int> lol;
    data>>lol;
    return lol;
}

int Client::getVersionOf(QString name){
    QSqlQuery query;
    query.exec("SELECT version FROM membre WHERE login='"+name+"'");
    query.next();
    return query.value(0).toInt();
}

Client::~Client(){
    for(QMap<QString, GRAmis*>::iterator it=myGroup.begin(); it!=myGroup.end(); it++) it.value()->disconnect(this);
    delete socket;

    qDebug()<<"Deconnection de"<<login;
}
