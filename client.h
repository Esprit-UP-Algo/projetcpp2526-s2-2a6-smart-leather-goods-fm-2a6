#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>

class Client
{
public:
    // Constructeurs
    Client();
    Client(QString cin, QString nom, QString tel, QString email, QString adresse, int points);
    Client(int id, QString cin, QString nom, QString tel, QString email, QString adresse, int points);
    // Getters (ajouter getPoints)
    int getId() const;
    QString getCin() const;
    QString getNom() const;
    QString getTel() const;
    QString getEmail() const;
    QString getAdresse() const;
    int getPoints() const;        // ← AJOUTER

    // Setters (ajouter setPoints)
    void setId(int id);
    void setCin(const QString& cin);
    void setNom(const QString& nom);
    void setTel(const QString& tel);
    void setEmail(const QString& email);
    void setAdresse(const QString& adresse);
    void setPoints(int points);    // ← AJOUTER

    // Méthodes CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* afficher();
    QSqlQueryModel* rechercher(const QString& critere);
    QSqlQueryModel* trierParPoints();  // ← AJOUTER

private:
    int id;
    QString cin;
    QString nom;
    QString tel;
    QString email;
    QString adresse;
    int points;    // ← AJOUTER
};
#endif // CLIENT_H   // ← Correction: ajout du commentaire
