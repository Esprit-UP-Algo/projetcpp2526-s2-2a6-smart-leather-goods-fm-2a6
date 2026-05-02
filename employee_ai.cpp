#include "employee_ai.h"

#include <algorithm>
#include <cmath>
#include <limits>
namespace {
static float clamp100(float v)
{
    if (v < 0.0f)
        return 0.0f;
    if (v > 100.0f)
        return 100.0f;
    return v;
}

struct PrototypeKnn
{
    QString classe;
    QString niveau;
    Employee rep;
};

static float distanceEuclidienne(const Employee &a, const Employee &b)
{
    // Normalisation [0..1] pour que tous les critères aient la même échelle.
    const float dx1 = (clamp100(a.experience) - clamp100(b.experience)) / 100.0f;
    const float dx2 = (clamp100(a.performance) - clamp100(b.performance)) / 100.0f;
    const float dx3 = (clamp100(a.formation) - clamp100(b.formation)) / 100.0f;
    const float dx4 = (clamp100(a.discipline) - clamp100(b.discipline)) / 100.0f;
    const float dx5 = (clamp100(a.productivite) - clamp100(b.productivite)) / 100.0f;
    return std::sqrt(dx1 * dx1 + dx2 * dx2 + dx3 * dx3 + dx4 * dx4 + dx5 * dx5);
}

static const QVector<PrototypeKnn> &jeuRepresentants()
{
    // 3 représentants par classe (S, A, B, C), comme demandé pour KNN.
    static const QVector<PrototypeKnn> reps = {
        { QStringLiteral("Talent strategique"), QStringLiteral("S"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 95, 96, 92, 93, 95 } },
        { QStringLiteral("Talent strategique"), QStringLiteral("S"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 91, 94, 90, 92, 94 } },
        { QStringLiteral("Talent strategique"), QStringLiteral("S"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 93, 90, 94, 91, 92 } },

        { QStringLiteral("Profil expert"), QStringLiteral("A"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 78, 82, 76, 80, 79 } },
        { QStringLiteral("Profil expert"), QStringLiteral("A"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 74, 79, 73, 77, 76 } },
        { QStringLiteral("Profil expert"), QStringLiteral("A"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 81, 75, 78, 74, 80 } },

        { QStringLiteral("Profil operationnel"), QStringLiteral("B"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 61, 65, 58, 62, 60 } },
        { QStringLiteral("Profil operationnel"), QStringLiteral("B"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 56, 60, 55, 59, 57 } },
        { QStringLiteral("Profil operationnel"), QStringLiteral("B"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 64, 57, 61, 56, 63 } },

        { QStringLiteral("Profil a risque"), QStringLiteral("C"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 42, 45, 38, 41, 40 } },
        { QStringLiteral("Profil a risque"), QStringLiteral("C"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 35, 40, 33, 38, 36 } },
        { QStringLiteral("Profil a risque"), QStringLiteral("C"),
          Employee{ QString(), QString(), QString(), QString(), QString(), QDate(), 0.0, 46, 37, 42, 35, 44 } }
    };
    return reps;
}

struct Voisin
{
    float distance = 0.0f;
    QString classe;
    QString niveau;
};

static QVector<Voisin> plusProchesVoisins(const Employee &e, int k)
{
    QVector<Voisin> voisins;
    const QVector<PrototypeKnn> &reps = jeuRepresentants();
    voisins.reserve(reps.size());
    for (const PrototypeKnn &p : reps) {
        Voisin v;
        v.distance = distanceEuclidienne(e, p.rep);
        v.classe = p.classe;
        v.niveau = p.niveau;
        voisins.push_back(v);
    }
    std::sort(voisins.begin(), voisins.end(), [](const Voisin &a, const Voisin &b) {
        return a.distance < b.distance;
    });
    if (k < 1)
        k = 1;
    if (voisins.size() > k)
        voisins.resize(k);
    return voisins;
}

static float scorePondereRegles(const Employee &e)
{
    const float exp = clamp100(e.experience);
    const float perf = clamp100(e.performance);
    const float form = clamp100(e.formation);
    const float disc = clamp100(e.discipline);
    const float prod = clamp100(e.productivite);
    return (exp * 0.25f) + (perf * 0.30f) + (form * 0.15f) + (disc * 0.15f) + (prod * 0.15f);
}

static QString classeDepuisScore(float score)
{
    const float s = clamp100(score);
    if (s >= 85.0f)
        return QStringLiteral("Talent strategique");
    if (s >= 70.0f)
        return QStringLiteral("Profil expert");
    if (s >= 50.0f)
        return QStringLiteral("Profil operationnel");
    return QStringLiteral("Profil a risque");
}

/// Lettre S/A/B/C (même logique que AIClassifier::classerNiveau sur le score pondéré).
static QString lettreNiveauDepuisEmploye(const Employee &e)
{
    const float s = clamp100(scorePondereRegles(e));
    if (s >= 85.0f)
        return QStringLiteral("S");
    if (s >= 70.0f)
        return QStringLiteral("A");
    if (s >= 50.0f)
        return QStringLiteral("B");
    return QStringLiteral("C");
}

static QString niveauDepuisClasse(const QString &classe)
{
    if (classe == QStringLiteral("Talent strategique"))
        return QStringLiteral("S");
    if (classe == QStringLiteral("Profil expert"))
        return QStringLiteral("A");
    if (classe == QStringLiteral("Profil operationnel"))
        return QStringLiteral("B");
    return QStringLiteral("C");
}

static QString profilDepuisLettreAbc(const QString &lettre)
{
    if (lettre == QStringLiteral("A"))
        return QStringLiteral("Profil expert");
    if (lettre == QStringLiteral("B"))
        return QStringLiteral("Profil operationnel");
    return QStringLiteral("Profil a risque");
}

static Employee prototypeLettre(const QString &lettre)
{
    const QVector<PrototypeKnn> &reps = jeuRepresentants();
    for (const PrototypeKnn &p : reps) {
        if (p.niveau == lettre)
            return p.rep;
    }
    return Employee{};
}

/// Premier employé de la base dont le niveau (S/A/B/C) correspond à `lettre` (hors selfId).
static Employee premiereReference(const QVector<Employee> &base, const QString &selfId, const QString &lettre)
{
    for (const Employee &emp : base) {
        if (!selfId.isEmpty() && emp.id == selfId)
            continue;
        if (lettreNiveauDepuisEmploye(emp) == lettre)
            return emp;
    }
    return Employee{};
}

static QString libelleReference(const Employee &e, bool depuisBase, const QString &lettreSlot)
{
    const QString nom = QStringLiteral("%1 %2").arg(e.nom, e.prenom).trimmed();
    if (depuisBase && !e.id.isEmpty())
        return QStringLiteral("%1 (ID %2)").arg(nom.isEmpty() ? QStringLiteral("(sans nom)") : nom, e.id);
    if (depuisBase && !nom.isEmpty())
        return QStringLiteral("%1 (ID %2)").arg(nom, e.id.isEmpty() ? QStringLiteral("—") : e.id);
    return QStringLiteral("Profil de secours %1 (aucun employé niveau %1 en base)").arg(lettreSlot);
}

static QString critereLePlusFaible(const Employee &e)
{
    struct C { const char *nom; float v; };
    const QVector<C> cs = {
        { "experience", clamp100(e.experience) },
        { "performance", clamp100(e.performance) },
        { "formation", clamp100(e.formation) },
        { "discipline", clamp100(e.discipline) },
        { "productivite", clamp100(e.productivite) }
    };
    const C *minC = &cs[0];
    for (const C &c : cs) {
        if (c.v < minC->v)
            minC = &c;
    }
    return QString::fromLatin1(minC->nom);
}

static QString recommandationPourProfil(const QString &classification, const Employee &e)
{
    const QString faible = critereLePlusFaible(e);
    if (classification == QStringLiteral("Profil a risque")) {
        return QStringLiteral("Plan d'accompagnement prioritaire axe sur %1").arg(faible);
    }
    if (classification == QStringLiteral("Profil operationnel")) {
        return QStringLiteral("Renforcement cible sur %1 pour progresser vers expert").arg(faible);
    }
    if (classification == QStringLiteral("Profil expert")) {
        return QStringLiteral("Mentorat et responsabilites avancees pour consolider le profil");
    }
    return QStringLiteral("Talent strategique a valoriser sur des missions critiques");
}
}

float AIClassifier::calculerScore(const Employee &e)
{
    // Score de proximité KNN : plus la distance moyenne aux 3 voisins est faible,
    // plus le score est élevé.
    const QVector<Voisin> knn = plusProchesVoisins(e, 3);
    if (knn.isEmpty())
        return 0.0f;

    float somme = 0.0f;
    for (const Voisin &v : knn)
        somme += v.distance;
    const float moyenne = somme / float(knn.size());
    // Distance max théorique dans 5D normalisé : sqrt(5).
    const float maxD = std::sqrt(5.0f);
    const float proximite = 1.0f - (moyenne / maxD);
    return clamp100(proximite * 100.0f);
}

QString AIClassifier::classerNiveau(float score)
{
    const float s = clamp100(score);
    if (s >= 85.0f)
        return QStringLiteral("S");
    if (s >= 70.0f)
        return QStringLiteral("A");
    if (s >= 50.0f)
        return QStringLiteral("B");
    return QStringLiteral("C");
}

QString AIClassifier::genererRecommandation(const Employee &e)
{
    const QString classe = genererClassification(calculerScore(e));
    const QString faible = critereLePlusFaible(e);
    if (classe == QStringLiteral("Profil a risque")) {
        return QStringLiteral("Plan d'accompagnement prioritaire axe sur %1").arg(faible);
    }
    if (classe == QStringLiteral("Profil operationnel")) {
        return QStringLiteral("Renforcement cible sur %1 pour progresser vers expert").arg(faible);
    }
    if (classe == QStringLiteral("Profil expert")) {
        return QStringLiteral("Mentorat et responsabilites avancees pour consolider le profil");
    }
    return QStringLiteral("Talent strategique a valoriser sur des missions critiques");
}

QString AIClassifier::genererClassification(float score)
{
    const float s = clamp100(score);
    if (s >= 85.0f)
        return QStringLiteral("Talent strategique");
    if (s >= 70.0f)
        return QStringLiteral("Profil expert");
    if (s >= 50.0f)
        return QStringLiteral("Profil operationnel");
    return QStringLiteral("Profil a risque");
}

ClassificationResult AIClassifier::analyser(const Employee &e)
{
    ClassificationResult r;
    const QVector<Voisin> knn = plusProchesVoisins(e, 3);

    // Vote pondéré inversement à la distance.
    QMap<QString, float> voteParClasse;
    QMap<QString, float> meilleureDist;
    for (const Voisin &v : knn) {
        const float w = 1.0f / (v.distance + 1e-4f);
        voteParClasse[v.classe] += w;
        if (!meilleureDist.contains(v.classe) || v.distance < meilleureDist[v.classe])
            meilleureDist[v.classe] = v.distance;
    }

    QString classeGagnante = QStringLiteral("Profil operationnel");
    float meilleurVote = -1.0f;
    float meilleureDistance = std::numeric_limits<float>::max();
    for (auto it = voteParClasse.constBegin(); it != voteParClasse.constEnd(); ++it) {
        const float vote = it.value();
        const float d = meilleureDist.value(it.key(), std::numeric_limits<float>::max());
        if (vote > meilleurVote || (qFuzzyCompare(vote + 1.0f, meilleurVote + 1.0f) && d < meilleureDistance)) {
            meilleurVote = vote;
            meilleureDistance = d;
            classeGagnante = it.key();
        }
    }

    r.score = calculerScore(e);
    r.classification = classeGagnante;
    if (classeGagnante == QStringLiteral("Talent strategique"))
        r.niveau = QStringLiteral("S");
    else if (classeGagnante == QStringLiteral("Profil expert"))
        r.niveau = QStringLiteral("A");
    else if (classeGagnante == QStringLiteral("Profil operationnel"))
        r.niveau = QStringLiteral("B");
    else
        r.niveau = QStringLiteral("C");
    r.recommandation = genererRecommandation(e);
    r.resume = QStringLiteral("%1 %2 - Niveau %3 (%4/100)")
                   .arg(e.nom, e.prenom, r.niveau, QString::number(std::round(r.score)));

    r.criteres.insert(QStringLiteral("Experience"), clamp100(e.experience));
    r.criteres.insert(QStringLiteral("Performance"), clamp100(e.performance));
    r.criteres.insert(QStringLiteral("Formation"), clamp100(e.formation));
    r.criteres.insert(QStringLiteral("Discipline"), clamp100(e.discipline));
    r.criteres.insert(QStringLiteral("Productivite"), clamp100(e.productivite));
    return r;
}

ClassificationResult AIClassifier::classifyWithThreeReferences(const Employee &current,
                                                                 const QVector<Employee> &base,
                                                                 const QString &selfId)
{
    ClassificationResult r;

    const QString lettres[3] = { QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C") };
    Employee refs[3];
    bool depuisBase[3] = { false, false, false };

    for (int i = 0; i < 3; ++i) {
        Employee cand = premiereReference(base, selfId, lettres[i]);
        const bool okBase = !cand.id.isEmpty() || !cand.nom.isEmpty() || !cand.prenom.isEmpty();
        if (okBase) {
            refs[i] = cand;
            depuisBase[i] = true;
        } else {
            refs[i] = prototypeLettre(lettres[i]);
            depuisBase[i] = false;
        }
    }

    const float dA = distanceEuclidienne(current, refs[0]);
    const float dB = distanceEuclidienne(current, refs[1]);
    const float dC = distanceEuclidienne(current, refs[2]);

    QString lettreGagnante = QStringLiteral("A");
    float distMin = dA;
    if (dB < distMin) {
        distMin = dB;
        lettreGagnante = QStringLiteral("B");
    }
    if (dC < distMin) {
        distMin = dC;
        lettreGagnante = QStringLiteral("C");
    }

    const int idxGagnant = (lettreGagnante == QStringLiteral("A")) ? 0 : (lettreGagnante == QStringLiteral("B") ? 1 : 2);

    const float maxD = std::sqrt(5.0f);
    r.score = clamp100((1.0f - (distMin / maxD)) * 100.0f);
    r.lettreFinale = lettreGagnante;
    r.classification = profilDepuisLettreAbc(lettreGagnante);
    r.niveau = lettreGagnante;
    r.recommandation = recommandationPourProfil(r.classification, current);

    r.refA_id = depuisBase[0] ? refs[0].id : QStringLiteral("—");
    r.refA_nom = QStringLiteral("%1 %2").arg(refs[0].nom, refs[0].prenom).trimmed();
    r.refB_id = depuisBase[1] ? refs[1].id : QStringLiteral("—");
    r.refB_nom = QStringLiteral("%1 %2").arg(refs[1].nom, refs[1].prenom).trimmed();
    r.refC_id = depuisBase[2] ? refs[2].id : QStringLiteral("—");
    r.refC_nom = QStringLiteral("%1 %2").arg(refs[2].nom, refs[2].prenom).trimmed();

    r.distanceA = dA;
    r.distanceB = dB;
    r.distanceC = dC;

    r.plusProcheId = depuisBase[idxGagnant] ? refs[idxGagnant].id : QStringLiteral("—");
    r.plusProcheNom = QStringLiteral("%1 %2").arg(refs[idxGagnant].nom, refs[idxGagnant].prenom).trimmed();

    const QString ligneA = libelleReference(refs[0], depuisBase[0], QStringLiteral("A"));
    const QString ligneB = libelleReference(refs[1], depuisBase[1], QStringLiteral("B"));
    const QString ligneC = libelleReference(refs[2], depuisBase[2], QStringLiteral("C"));

    r.preuveAbc = QStringLiteral(
                      "Base de référence (1 employé par niveau A, B, C) :\n"
                      "• A : %1\n"
                      "• B : %2\n"
                      "• C : %3\n\n"
                      "Distances (espace 5 critères normalisé) :\n"
                      "• d(A) = %4\n"
                      "• d(B) = %5\n"
                      "• d(C) = %6\n\n"
                      "Décision :\n"
                      "• distance minimale = %7\n"
                      "• référence la plus proche = %8 (ID %9)\n"
                      "• classe finale = %10 (%11)")
                      .arg(ligneA,
                           ligneB,
                           ligneC,
                           QString::number(dA, 'f', 3),
                           QString::number(dB, 'f', 3),
                           QString::number(dC, 'f', 3),
                           QString::number(distMin, 'f', 3),
                           r.plusProcheNom.isEmpty() ? QStringLiteral("(profil de secours)") : r.plusProcheNom,
                           r.plusProcheId,
                           lettreGagnante,
                           r.classification);

    r.resume = QStringLiteral("%1 %2 — distance minimale vers référence %3 (%4/100)")
                   .arg(current.nom,
                        current.prenom,
                        lettreGagnante,
                        QString::number(std::round(r.score)));

    r.criteres.insert(QStringLiteral("Experience"), clamp100(current.experience));
    r.criteres.insert(QStringLiteral("Performance"), clamp100(current.performance));
    r.criteres.insert(QStringLiteral("Formation"), clamp100(current.formation));
    r.criteres.insert(QStringLiteral("Discipline"), clamp100(current.discipline));
    r.criteres.insert(QStringLiteral("Productivite"), clamp100(current.productivite));
    return r;
}

ClassificationResult AIClassifier::analyserAvecBase(const Employee &e, const QVector<Employee> &base, const QString &selfId)
{
    return classifyWithThreeReferences(e, base, selfId);
}

ClassificationResult AIClassifier::simulerFormation(const Employee &e, float gainFormation)
{
    Employee simule = e;
    simule.formation = clamp100(simule.formation + gainFormation);
    return analyser(simule);
}

QVector<Employee> AIClassifier::topEmployes(const QVector<Employee> &employes, int limite)
{
    QVector<Employee> tri = employes;
    std::sort(tri.begin(), tri.end(), [](const Employee &a, const Employee &b) {
        return AIClassifier::calculerScore(a) > AIClassifier::calculerScore(b);
    });
    if (limite < 0)
        limite = 0;
    if (tri.size() > limite)
        tri.resize(limite);
    return tri;
}

QVector<Employee> AIClassifier::employesARisque(const QVector<Employee> &employes, float seuil)
{
    QVector<Employee> out;
    for (const Employee &e : employes) {
        if (calculerScore(e) < seuil)
            out.push_back(e);
    }
    std::sort(out.begin(), out.end(), [](const Employee &a, const Employee &b) {
        return AIClassifier::calculerScore(a) < AIClassifier::calculerScore(b);
    });
    return out;
}
