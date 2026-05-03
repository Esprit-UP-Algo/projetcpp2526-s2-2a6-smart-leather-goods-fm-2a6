# Qt C++ Integration (Historique de Mode)

```cpp
// Dans MainWindow::showHistoriqueModeDialog()
QNetworkRequest req(QUrl("http://127.0.0.1:8010/predict?year=2032"));
auto* reply = networkManager->get(req);
connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const auto data = reply->readAll();
    const auto doc = QJsonDocument::fromJson(data);
    const auto obj = doc.object();

    const QString summary = obj.value("summary").toString();
    const auto styles = obj.value("top_styles").toArray();
    // injecter dans QLabel/QTableWidget de l'onglet Historique de Mode

    reply->deleteLater();
});
```

Champs utiles a afficher dans le module Produit:
- `summary`
- `top_styles`
- `color_palette`
- `fabrics_materials`
- `silhouettes`
- `confidence`
- `recommended_product_attributes`
