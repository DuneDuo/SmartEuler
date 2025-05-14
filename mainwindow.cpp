// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkRequest>
#include <QFileDialog>
#include <QBuffer>
#include <QMessageBox>
#include <QDateTime>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QUuid>
#include <memory>
#include <QThreadPool>
#include <atomic>

const QString MainWindow::SIMPLETEX_API_URL = "https://server.simpletex.cn/api/latex_ocr";
const QString MainWindow::USER_ACCESS_TOKEN = "b6DYZDdwDWf71lciobdCVMR3ynfrxa5wWqpXitrXMLMP9NpI8WPWb1ia1PDVHPh0";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    ui->resultLabel->setTextInteractionFlags(Qt::TextSelectableByMouse); // 允许选择文本
    ui->resultLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; }");
    connect(ui->captureButton, &QPushButton::clicked, this, &MainWindow::onCaptureClicked);
    connect(ui->btnUpload, &QPushButton::clicked, this, &MainWindow::onUploadClicked);
    setAcceptDrops(true);
    QFont font("Consolas", 10); // 使用等宽字体
    font.setStyleHint(QFont::Monospace);
    ui->resultLabel->setFont(font);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 修改后的handleResponse函数
void MainWindow::handleResponse(const QByteArray &responseData)
{
    qDebug() << "收到响应数据：" << responseData;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        showError(QString("JSON解析失败: %1").arg(parseError.errorString()));
        return;
    }

    QJsonObject result = doc.object();
    qDebug() << "完整JSON结构：" << result; // 调试输出

    if(result["status"].toBool()) {
        // 修正路径：res -> latex
        QJsonValue res = result.value("res");
        if(res.isObject()){
            QString latex = res.toObject().value("latex").toString();
            if(!latex.isEmpty()){
                showResult(latex);
            } else {
                showError("响应中缺少latex字段");
            }
        } else {
            showError("响应格式错误：res不是对象");
        }
    } else {
        showError(result["message"].toString());
    }
}


cv::Mat MainWindow::preprocessImage(const QImage &image)
{
    try {
        if (image.isNull()) {
            throw std::runtime_error("输入图像为空");
        }

        // 转换为RGB888格式
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
        if(rgbImage.isNull()) {
            throw std::runtime_error("RGB转换失败");
        }

        // 创建OpenCV矩阵
        cv::Mat mat(rgbImage.height(), rgbImage.width(), CV_8UC3,
                    const_cast<uchar*>(rgbImage.bits()),
                    static_cast<size_t>(rgbImage.bytesPerLine()));

        // 尺寸校验
        if(mat.cols < 32 || mat.rows < 32) {
            throw std::runtime_error("图像尺寸过小（至少32x32像素）");
        }

        // 降采样处理
        cv::Mat resized;
        double scale = std::min(800.0 / mat.cols, 800.0 / mat.rows);
        if(scale < 1.0) {
            cv::resize(mat, resized, cv::Size(), scale, scale, cv::INTER_AREA);
        } else {
            resized = mat.clone();
        }

        // 灰度化处理
        cv::Mat gray;
        cv::cvtColor(resized, gray, cv::COLOR_RGB2GRAY);

        // 锐化处理
        cv::Mat sharpened;
        cv::GaussianBlur(gray, sharpened, cv::Size(0, 0), 3);
        cv::addWeighted(gray, 1.2, sharpened, -0.2, 0, sharpened);
        // 二值化处理
        cv::Mat binary;
        cv::threshold(sharpened, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        cv::adaptiveThreshold(sharpened, binary, 255,
                              cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY, 11, 2);

        // 确保矩阵连续性
        if(!binary.isContinuous()) {
            return binary.clone();
        }
        return binary;
    } catch (const cv::Exception& e) {
        showError(QString("OpenCV错误: %1").arg(e.what()));
        return cv::Mat();
    } catch (const std::exception& e) {
        showError(QString("预处理错误: %1").arg(e.what()));
        return cv::Mat();
    }
}
void MainWindow::imageCleanupHandler(void* matPtr)
{
    delete static_cast<std::shared_ptr<cv::Mat>*>(matPtr);
}
void MainWindow::processImage(const QImage& image)
{
    qDebug() << "开始处理图像...";

    // 预处理图像
    cv::Mat processed = preprocessImage(image);
    if(processed.empty()) {
        showError("预处理失败");
        return;
    }

    // 使用智能指针管理矩阵生命周期
    auto matHolder = std::make_shared<cv::Mat>(processed);
    auto* matHolderPtr = new std::shared_ptr<cv::Mat>(matHolder);

    QImage cvImage(
        (*matHolderPtr)->data,
        (*matHolderPtr)->cols,
        (*matHolderPtr)->rows,
        static_cast<qsizetype>((*matHolderPtr)->step), // 关键修改：使用qsizetype
        QImage::Format_Grayscale8,
        &MainWindow::imageCleanupHandler, // 使用静态成员函数
        matHolderPtr // 传递智能指针包装器
        );

    if(cvImage.isNull()) {
        showError("无法创建Qt图像");
        return;
    }

    // 编码为JPEG
    QByteArray imageData;
    QBuffer buffer(&imageData);
    if(buffer.open(QIODevice::WriteOnly)) {
        if(!cvImage.copy().save(&buffer, "JPEG", 70)) { // 深拷贝
            showError("图像编码失败");
            return;
        }
    } else {
        showError("无法打开缓冲区");
        return;
    }
    struct RequestContext {
        QAtomicInt valid{1};
        QNetworkReply* reply;
        QTimer* timer;
        QByteArray response;
    };
    // 构造multipart请求
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    try {
        // 生成参数（需包含所有非文件参数）
        QMap<QString, QString> params{
            {"math", "true"},
            {"formula_type", "0"}
        };

        // 设置请求头
        QNetworkRequest request;
        request.setUrl(QUrl(SIMPLETEX_API_URL));
        request.setRawHeader("token", USER_ACCESS_TOKEN.toUtf8()); // 关键修改：UAT鉴权头
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArray("multipart/form-data; boundary=") + multiPart->boundary());

        // 添加文件部分
        QHttpPart imagePart;
        imagePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/jpeg");
        imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                            "form-data; name=\"file\"; filename=\"formula.jpg\"");
        imagePart.setBody(imageData);
        multiPart->append(imagePart);

        // 添加其他参数（必须与签名参数一致）
        foreach (const QString& key, params.keys()) {
            QHttpPart part;
            part.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QString("form-data; name=\"%1\"").arg(key));
            part.setBody(params[key].toUtf8());
            multiPart->append(part);
        }
        QNetworkReply* reply = manager->post(request, multiPart);
        multiPart->setParent(reply);

        // 使用C++11原子类型
        struct RequestContext {
            std::atomic<bool> valid{true};
            QPointer<QNetworkReply> reply;
            QTimer* timer;
            QByteArray response;
        };

        auto* context = new RequestContext;
        context->reply = reply;
        context->timer = new QTimer;

        // 超时处理（10秒）
        context->timer->setSingleShot(true);
        QObject::connect(context->timer, &QTimer::timeout, [=]() {
            if (context->valid.load() && context->reply && context->reply->isRunning()) {
                context->reply->abort();
                showError("请求超时");
            }
            context->valid.store(false);
            context->timer->deleteLater();
            delete context;
        });

        // 响应处理
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            context->timer->stop();

            if (!context->valid.load() || !context->reply) {
                delete context;
                return;
            }

            const QByteArray response = context->reply->readAll();

            // 使用Qt线程池处理响应
            QtConcurrent::run([=]() {
                if (!context->valid.load()) {
                    delete context;
                    return;
                }

                // 主线程更新UI
                QMetaObject::invokeMethod(this, [=]() {
                    if (context->valid.load()) {
                        handleResponse(response);
                    }
                    context->valid.store(false);
                    delete context;
                }, Qt::QueuedConnection);
            });
        });

        context->timer->start(10000);
        reply->setParent(nullptr); // 解除父子关系

    } catch (const std::exception& e) {
        multiPart->deleteLater();
        showError(QString("请求构造异常: %1").arg(e.what()));
    }
}

// 修改后的错误处理逻辑



void MainWindow::showResult(const QString &latex)
{
    // 转换Unicode转义字符（如\u786e -> 确）
    QTextDocument textDoc;
    textDoc.setHtml(latex);
    QString decoded = textDoc.toPlainText();

    // 显示到界面
    ui->resultLabel->setText(QString("识别结果：\n%1").arg(decoded));
    ui->resultLabel->setWordWrap(true); // 启用自动换行

    // 调试输出
    qDebug() << "原始LaTeX：" << latex;
    qDebug() << "解码后文本：" << decoded;
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    statusBar()->showMessage(message, 5000);
}
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        processImage(QImage(urls.first().toLocalFile()));
    }
}
    void MainWindow::onUploadClicked()
    {
        QFileDialog dialog(this);
        dialog.setOption(QFileDialog::DontUseNativeDialog);
        dialog.setNameFilter("图片文件 (*.png *.jpg *.bmp)");

        // 修正路径设置方式
        QString defaultDir = QDir::homePath(); // 使用标准路径
        dialog.setDirectory(defaultDir);       // 明确使用QString参数

        if (dialog.exec()) {
            QStringList files = dialog.selectedFiles();
            if (!files.isEmpty()) {
                QString filePath = files.first();
                QImage image(filePath);
                if (!image.isNull()) {
                    processImage(image);
                } else {
                    showError("无法加载图片: " + filePath);
                }
            }
        }
    }



    void MainWindow::onCaptureClicked()
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        processImage(screen->grabWindow(0).toImage());
    }
