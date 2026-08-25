#include <QApplication>
#include <QScreen>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QDir>
#include <QThread>
#include "main_window.h"
#include "../theme/theme.h"
#include "../splash/splash_screen.h"
#include "../dialogs/startup_dialog.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("FreeEffect");
    app.setOrganizationName("FreeEffect");
    app.setApplicationVersion("0.1.0");
    app.setWindowIcon(QIcon(":/app/icon.svg"));
    
    // Apply AE dark theme
    FreeEffect::Theme::applyTheme(&app);
    
    // Show splash screen
    QPixmap splashPixmap(":/splash/splash.png");
    if (splashPixmap.isNull()) {
        splashPixmap = QPixmap(800, 450);
        splashPixmap.fill(QColor(26, 26, 46));
    }
    
    FreeEffect::SplashScreen splash(splashPixmap);
    splash.show();
    QApplication::processEvents();
    
    splash.showMessage("Loading core modules...");
    QApplication::processEvents();
    QThread::msleep(300);
    
    splash.showMessage("Initializing UI...");
    QApplication::processEvents();
    QThread::msleep(300);
    
    splash.showMessage("Loading resources...");
    QApplication::processEvents();
    QThread::msleep(200);
    
    // Show startup dialog
    FreeEffect::StartupDialog startupDialog;
    splash.finish(&startupDialog);
    startupDialog.setWindowIcon(QIcon(":/app/icon.svg"));
    
    int result = startupDialog.exec();
    
    FreeEffect::MainWindow* window = nullptr;
    
    if (startupDialog.getResult() == FreeEffect::StartupDialog::NewProject) {
        window = new FreeEffect::MainWindow();
        window->show();
    } else if (startupDialog.getResult() == FreeEffect::StartupDialog::OpenProject) {
        window = new FreeEffect::MainWindow();
        window->show();
        QTimer::singleShot(100, window, &FreeEffect::MainWindow::onImportFile);
    } else if (startupDialog.getResult() == FreeEffect::StartupDialog::OpenRecent) {
        window = new FreeEffect::MainWindow();
        window->show();
        window->onOpenProject(startupDialog.getSelectedRecent());
    } else {
        return 0;
    }
    
    return app.exec();
}
