#ifndef CLASSIFICATIONIAWIDGET_H
#define CLASSIFICATIONIAWIDGET_H

#include <QDate>
#include <QJsonObject>
#include <QWidget>

class QLabel;
class QPushButton;
class QFrame;
class QNetworkAccessManager;
class QNetworkReply;
class KnnGraphWidget;

class ClassificationIAWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClassificationIAWidget(QWidget *parent = nullptr);
    void setEmployeeId(int employeeId);
    int employeeId() const { return m_employeeId; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct EmployeeData {
        int id = -1;
        QString nom;
        QString prenom;
        QString poste;
        QString departement;
        double salaire = 0.0;
        QDate dateEmbauche;
        int anciennete = 0;
    };

    bool loadEmployeeData(int id);
    void requestFullAnalysis();
    void sendGroqClassification();
    void handleLlmApiReply(QNetworkReply *reply);
    QJsonObject parseResponseJson(QString content) const;
    void updateUI(const QJsonObject &o);
    void setLoading(bool loading);
    void updateResumeSection(const QJsonObject &o);
    void resetResumePlaceholder();
    QString buildUserMessage() const;
    /** Message court dans CONTEXTE ; détail technique tronqué dans la barre du haut. */
    void setAnalysisErrorUi(const QString &contextShortFr, const QString &technicalDetail = QString());

    void showDetailsDialog();
    void showCompareDialog();

    EmployeeData m_emp;
    int m_employeeId = -1;
    bool m_loading = false;

    QLabel *m_badgeTop = nullptr;
    QLabel *m_titleAtlas = nullptr;
    QFrame *m_sepHeader = nullptr;
    QLabel *m_employeeNameLine = nullptr;
    QLabel *m_deptBadge = nullptr;
    QLabel *m_quickScoreBadge = nullptr;
    QLabel *m_badgeClasse = nullptr;
    QLabel *m_badgeScore = nullptr;
    QLabel *m_badgeNiveau = nullptr;
    KnnGraphWidget *m_knnGraph = nullptr;
    QLabel *m_badgeClassLetter = nullptr;
    QPushButton *m_btnFullAnalysis = nullptr;
    QPushButton *m_btnDetails = nullptr;
    QPushButton *m_btnCompare = nullptr;
    QLabel *m_statusLabel = nullptr;

    QWidget *m_resumeInner = nullptr;
    QLabel *m_resumeTitle = nullptr;
    QLabel *m_lblScoreGlobalRow = nullptr;
    QLabel *m_lblNiveauRow = nullptr;
    QLabel *m_lblClasseRow = nullptr;
    QLabel *m_lblActionRow = nullptr;
    QFrame *m_resumeSep = nullptr;
    QLabel *m_lblPreuveTitle = nullptr;
    QLabel *m_lblPreuveBody = nullptr;
    QLabel *m_lblVoisinsTitle = nullptr;
    QLabel *m_lblVoisinsBody = nullptr;

    QJsonObject m_lastResult;

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_activeReply = nullptr;
};

#endif
