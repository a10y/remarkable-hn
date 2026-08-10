#include "hnbackend.h"

#include <QGuiApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <memory>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

struct MxcfbRect {
    std::uint32_t top;
    std::uint32_t left;
    std::uint32_t width;
    std::uint32_t height;
};

struct MxcfbAltBufferData {
    std::uint32_t physicalAddress;
    std::uint32_t width;
    std::uint32_t height;
    MxcfbRect updateRegion;
};

struct MxcfbUpdateData {
    MxcfbRect updateRegion;
    std::uint32_t waveformMode;
    std::uint32_t updateMode;
    std::uint32_t updateMarker;
    int temperature;
    std::uint32_t flags;
    int ditherMode;
    int quantBit;
    MxcfbAltBufferData altBufferData;
};

constexpr unsigned long mxcfbSendUpdate = _IOW('F', 0x2e, MxcfbUpdateData);

void enableAppLoadRefreshes(QQuickWindow *window)
{
    if (!window || !qEnvironmentVariableIsSet("QTFB_KEY"))
        return;

    const int framebuffer = ::open("/dev/fb0", O_RDWR);
    if (framebuffer < 0) {
        qWarning() << "Could not open the AppLoad framebuffer for refreshes";
        return;
    }

    auto elapsed = std::make_shared<QElapsedTimer>();
    elapsed->start();
    QObject::connect(window, &QQuickWindow::frameSwapped, window,
                     [window, framebuffer, elapsed] {
        if (elapsed->elapsed() < 40)
            return;
        elapsed->restart();

        static std::uint32_t marker = 1;
        MxcfbUpdateData update{};
        update.updateRegion.width = static_cast<std::uint32_t>(window->width());
        update.updateRegion.height = static_cast<std::uint32_t>(window->height());
        update.waveformMode = 3; // GL16/UI refresh through the AppLoad shim.
        update.updateMarker = marker++;
        ::ioctl(framebuffer, mxcfbSendUpdate, &update);
    }, Qt::DirectConnection);
}

} // namespace

int main(int argc, char *argv[])
{
    bool smokeTest = false;
    for (int index = 1; index < argc; ++index)
        smokeTest = smokeTest || std::strcmp(argv[index], "--smoke-test") == 0;
    if (smokeTest)
        qputenv("QT_QPA_PLATFORM", "offscreen");

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("HN Reader"));
    app.setOrganizationName(QStringLiteral("remarkable-qt-app"));

    HnBackend backend;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("HnReader"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
        return -1;

    enableAppLoadRefreshes(qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst()));

    if (smokeTest) {
        QObject::connect(&backend, &HnBackend::loadingStoriesChanged, &app, [&backend] {
            if (backend.loadingStories())
                return;
            if (backend.stories().size() == 10) {
                qInfo() << "Smoke test loaded" << backend.stories().size() << "stories";
                QCoreApplication::exit(0);
            } else {
                qCritical() << "Smoke test failed:" << backend.errorMessage();
                QCoreApplication::exit(2);
            }
        });
        QTimer::singleShot(25000, &app, [] {
            qCritical() << "Smoke test timed out";
            QCoreApplication::exit(3);
        });
    }

    backend.refreshStories();
    return app.exec();
}
