// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpMultiPart>  // 添加这行
#include <QHttpPart>       // 添加这行
#include <QScreen>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUrlQuery>
#include <QUuid>
#include <QJsonArray>
#include <QStatusBar>
#include <QDragEnterEvent>  // 新增
#include <QDropEvent>       // 新增
#include <QMimeData>        // 新增
#include <QUrl>             // 新增
#include <QStandardPaths> // 新增
#include <QTimer>  // 新增头文件
#include <QDebug>
#include <QPointer>
#include <QtConcurrent/QtConcurrent>
#include <atomic>
#include <opencv2/core.hpp>  // 确保包含cv::Size定义
// OpenCV头文件必须在前
#include <opencv2/opencv.hpp>

inline QDebug operator<<(QDebug debug, const cv::Size_<int>& size)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "cv::Size(" << size.width << ", " << size.height << ")";
    return debug;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static const QString USER_ACCESS_TOKEN;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onCaptureClicked();
    void onUploadClicked();
    void handleResponse(const QByteArray& responseData); // 添加参数

private:
    // SimpleTex配置[2][9]
    Ui::MainWindow *ui;
    QTimer* requestTimer;
    QNetworkAccessManager *manager;
    int retryCount = 0;
    static const QString SIMPLETEX_API_URL;
    static void imageCleanupHandler(void* matPtr);

    // 修改请求处理函数
    QByteArray generateSimpleTexSign(const QMap<QString, QString>& params);
    QString generateHMAC(const QString& nonce, const QString& timestamp);
    cv::Mat preprocessImage(const QImage &image);
    QByteArray imageToByteArray(const cv::Mat &mat);
    void processImage(const QImage &image);
    QString postProcessLatex(const QString& raw);
    void showResult(const QString &latex); // 确保已存在声明
    void showError(const QString &message); // 确保已存在声明
};

#endif // MAINWINDOW_H
