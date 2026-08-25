#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("FreeEffect");
    app.setOrganizationName("FreeEffect");
    app.setApplicationVersion("0.1.0");
    
    FreeEffect::MainWindow window;
    window.show();
    
    return app.exec();
}
