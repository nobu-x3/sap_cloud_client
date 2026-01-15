#include <QApplication>
#include <QStyleFactory>
#include "sap_cloud_client/main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("SapCloud");
    app.setOrganizationName("SapCloud");
    app.setApplicationVersion("0.1.0");

    // Use Fusion style for consistent cross-platform look
    app.setStyle(QStyleFactory::create("Fusion"));

    sap::client::MainWindow window;
    window.show();

    return app.exec();
}
