#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QGraphicsLineItem>
#include <QPen>
#include <QDebug>
#include <QRectF>
#include <QTransform>
#include <QPolygonF>
#include <QResizeEvent>
#include <cmath>
#include <QPainterPath>
#include <QResource>
#include <QDir>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 初始化场景
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    ui->graphicsView->setFrameShape(QFrame::NoFrame); // 移除默认边框

    scene_2 = new QGraphicsScene(this);
    ui->graphicsView_2->setScene(scene_2);
    ui->graphicsView_2->setStyleSheet("border: 1px solid black;"); // 设置黑色边框（1px实线）
    ui->graphicsView_2->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    ui->graphicsView_2->setRenderHint(QPainter::SmoothPixmapTransform, true); // 平滑缩放
    
    // 设置初始中心点为0,0
    carPosition = QPointF(0, 0);
    
    // 创建小车组
    carGroup = new QGraphicsItemGroup();
    scene->addItem(carGroup);
    
    // 创建小车车身（中心点在矩形中心）
    carBody = new QGraphicsRectItem(-CAR_LENGTH/2, -CAR_WIDTH/2, CAR_LENGTH, CAR_WIDTH);
    carBody->setBrush(QColor(100, 150, 255)); // 浅蓝色车身
    carBody->setPen(QPen(Qt::black, 1)); // 黑色边框
    carGroup->addToGroup(carBody);
    
    // 创建车头指示器
    createCarHeadIndicator();
    
    // 设置小车位置和方向（初始方向0°指向右上方）
    carGroup->setPos(carPosition);
    carGroup->setRotation(carDirection);
    
    // 初始化轨迹点
    trajectory.append(carPosition);
    
    // 连接按钮信号
    connect(ui->btnLeft, &QPushButton::pressed, this, &MainWindow::onLeftPressed);
    connect(ui->btnRight, &QPushButton::pressed, this, &MainWindow::onRightPressed);
    connect(ui->btnAccel, &QPushButton::pressed, this, &MainWindow::onAccelPressed);
    connect(ui->btnDecel, &QPushButton::pressed, this, &MainWindow::onDecelPressed);
    connect(ui->btnBrake, &QPushButton::pressed, this, &MainWindow::onBrakePressed);
    connect(ui->btnFigure8, &QPushButton::pressed, this, &MainWindow::onFigure8Pressed); // 8字形按钮
    connect(ui->btnFigureHandWrite, &QPushButton::pressed, this, &MainWindow::onFigureHandWritePressed); // 8字形按钮
    connect(ui->loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(ui->initButton, &QPushButton::clicked, this, &MainWindow::onInitPressed);

    // 按钮释放连接
    connect(ui->btnLeft, &QPushButton::released, this, &MainWindow::releaseControls);
    connect(ui->btnRight, &QPushButton::released, this, &MainWindow::releaseControls);
    connect(ui->btnAccel, &QPushButton::released, this, &MainWindow::releaseControls);
    connect(ui->btnDecel, &QPushButton::released, this, &MainWindow::releaseControls);
    
    // 初始化定时器（更新频率30Hz）
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateCarPosition);
    timer->start(33); // ≈30fps
    
    // 初始状态显示
    updateStatusDisplay();
    
    // 创建四个角的坐标标签
    QFont labelFont("Arial", 10);
    for (int i = 0; i < 4; i++) {
        cornerLabels[i] = new QGraphicsSimpleTextItem();
        cornerLabels[i]->setBrush(Qt::black);
        cornerLabels[i]->setFont(labelFont);
        cornerLabels[i]->setZValue(10); // 确保在最上层
        scene->addItem(cornerLabels[i]);
    }
    
    // 创建视图边框
    viewBorder = new QGraphicsRectItem();
    viewBorder->setPen(QPen(Qt::darkGray, 2));
    viewBorder->setBrush(Qt::NoBrush);
    viewBorder->setZValue(5); // 在轨迹之上，小车之下
    scene->addItem(viewBorder);

    viewBorder_2 = new QGraphicsRectItem();
    viewBorder_2->setPen(QPen(Qt::darkGray, 2));
    viewBorder_2->setBrush(Qt::NoBrush);
    viewBorder_2->setZValue(5); // 在轨迹之上，小车之下
    scene_2->addItem(viewBorder_2);

    // 初始设置视图中心为(0,0)
    ui->graphicsView->centerOn(0, 0);
    
    // 初始更新坐标标签和边框
    updateCornerCoordinates();
    updateViewBorder();
    
    // 设置初始场景范围（以小车为中心）
    updateSceneRect();
}

void MainWindow::onInitPressed()
{
    driveMode = manualMode;
    carPosition = QPointF(0, 0);
    carDirection = 0;
    carSpeed = 0;
    trajectory.clear();
}

// 动态更新场景范围
void MainWindow::updateSceneRect()
{
    // 获取当前视图范围
    QRectF viewRect = ui->graphicsView->mapToScene(
        ui->graphicsView->viewport()->rect()
    ).boundingRect();
    
    // 计算新的场景范围（比视图范围大一些）
    double padding = 500; // 场景边界留白
    QRectF newSceneRect(
        carPosition.x() - padding,
        carPosition.y() - padding,
        viewRect.width() + 2 * padding,
        viewRect.height() + 2 * padding
    );
    
    // 设置新的场景范围
    scene->setSceneRect(newSceneRect);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // 窗口大小改变后更新坐标标签和边框
    updateCornerCoordinates();
    updateViewBorder();
}

void MainWindow::createCarHeadIndicator()
{
    // 创建三角形车头指示器
    QPolygonF headPolygon;
    headPolygon << QPointF(CAR_LENGTH/2, 0)     // 顶点（车头尖端）
                << QPointF(CAR_LENGTH/2 - 20, -10) // 左点
                << QPointF(CAR_LENGTH/2 - 20, 10);  // 右点
    
    carHead = new QGraphicsPolygonItem(headPolygon);
    carHead->setBrush(Qt::red); // 红色车头指示器
    carHead->setPen(Qt::NoPen);
    carGroup->addToGroup(carHead);
}

void MainWindow::onLeftPressed()
{
    leftPressed = true;
    driveMode = manualMode;
}

void MainWindow::onRightPressed()
{
    rightPressed = true;
    driveMode = manualMode;
}

void MainWindow::onAccelPressed()
{
    accelPressed = true;
    driveMode = manualMode;
}

void MainWindow::onDecelPressed()
{
    decelPressed = true;
    driveMode = manualMode;
}

void MainWindow::onBrakePressed()
{
    carSpeed = 0;  // 急刹停车
    driveMode = manualMode;
}

void MainWindow::onFigure8Pressed()
{
    driveMode = figure8Mode; // 切换8字形模式
    
    // 以当前小车位置为起点，当前方向为起始方向生成8字形
    generateFigure8();
    
    figureIndex = 1;
    carSpeed = 5; // 设置固定速度
}

void MainWindow::onFigureHandWritePressed()
{
    driveMode = figureHandWriteMode; // 切换手写模式
    
    figureIndex = 1;
    carSpeed = 5; // 设置固定速度
}

void MainWindow::releaseControls()
{
    leftPressed = false;
    rightPressed = false;
    accelPressed = false;
    decelPressed = false;
}

void MainWindow::generateFigure8()
{
    // 生成8字形轨迹点（参数方程）
    int totalPoints = 200; // 总点数
    figurePoints.clear();

    // 标准8字形参数方程（以原点为中心）
    for(int i = 0; i < totalPoints; i++) {
        double t = 2.0 * M_PI * i / totalPoints;
        double x_orig = figure8Size * sin(t) / (1 + pow(cos(t), 2));
        double y_orig = figure8Size * sin(t) * cos(t) / (1 + pow(cos(t), 2));
        figurePoints.append(QPointF(x_orig, y_orig));

        // 应用旋转（使曲线在t=0时与x轴相切）
        // double x = x_orig * cos(roAngle) - y_orig * sin(roAngle);
        // double y = x_orig * sin(roAngle) + y_orig * cos(roAngle);
        // figurePoints.append(QPointF(x, y));
    }

    displayPoints();
    adjustFigure();
}

void MainWindow::adjustFigure()
{
    double dx = figurePoints[1].x() - figurePoints[0].x();
    double dy = figurePoints[1].y() - figurePoints[0].y();
    
    // 使用 atan2 计算弧度角（范围：[-π, π]）
    double angleRad = std::atan2(dy, dx);

    // 计算旋转角度（使曲线在起点处与x轴相切）
    double roAngle = -angleRad;

    for(int i = 0; i < figurePoints.size(); i++) {
        double x_orig = figurePoints[i].x();
        double y_orig = figurePoints[i].y();

        // 应用旋转（使曲线在t=0时与x轴相切）
        figurePoints[i].setX(x_orig * cos(roAngle) - y_orig * sin(roAngle));
        figurePoints[i].setY(x_orig * sin(roAngle) + y_orig * cos(roAngle));
    }

    // 计算旋转角度（使8字形方向与小车当前方向一致）
    double rotationAngle = carDirection;
    
    // 创建变换矩阵
    QTransform transform;
    transform.translate(carPosition.x(), carPosition.y()); // 平移到小车当前位置
    transform.rotate(rotationAngle); // 旋转到小车当前方向
    
    // 应用变换到所有点
    for(int i = 0; i < figurePoints.size(); i++) {
        figurePoints[i] = transform.map(figurePoints[i]);
    }
}

void MainWindow::updateCarPosition()
{
    switch (driveMode)
    {
    case figure8Mode:
        if(figureIndex < figurePoints.size()) {
            // 获取下一个8字形点
            QPointF nextPos = figurePoints[figureIndex];
            
            // 计算方向变化（基于当前位置和下一位置）
            double dx = nextPos.x() - carPosition.x();
            double dy = nextPos.y() - carPosition.y();
            carDirection = qRadiansToDegrees(std::atan2(dy, dx));
            
            // 更新位置
            carPosition = nextPos;
            figureIndex++;
            
            // 如果到达终点，回到起点
            if(figureIndex >= figurePoints.size()) {
                figureIndex = 0;
            }
        }
        break;
    case figureHandWriteMode:
        if(figureIndex < figurePoints.size()) {
            // 获取下一个点
            QPointF nextPos = figurePoints[figureIndex];
            
            // 计算方向变化（基于当前位置和下一位置）
            double dx = nextPos.x() - carPosition.x();
            double dy = nextPos.y() - carPosition.y();
            carDirection = qRadiansToDegrees(std::atan2(dy, dx));
            
            // 更新位置
            carPosition = nextPos;
            figureIndex++;
            
            // 如果到达终点，回到起点
            if(figureIndex >= figurePoints.size()) {
                driveMode = manualMode;
                carSpeed = 0;
            }
        }
        break;
    default:
        // 手动模式
        // 转向控制（左右转向）
        if (leftPressed) carDirection -= 2.0;  // 左转
        if (rightPressed) carDirection += 2.0; // 右转
        
        // 速度控制（加速/减速）
        if (accelPressed) carSpeed += 0.2;
        if (decelPressed) carSpeed -= 0.2;
        if (carSpeed < 0) carSpeed = 0; // 最低速度为0
        
        // 限制最大速度
        if (carSpeed > 10) carSpeed = 10;
        
        // 计算位移增量（极坐标转换）
        double rad = qDegreesToRadians(carDirection);
        double dx = carSpeed * cos(rad);
        double dy = carSpeed * sin(rad);
        
        // 更新位置
        carPosition += QPointF(dx, dy);
        break;
    }
    
    // 更新小车位置和方向
    carGroup->setPos(carPosition);
    carGroup->setRotation(carDirection);

    // 更新场景范围
    updateSceneRect();
    
    // 确保视图中心跟随小车
    centerViewOnCar();
    
    // 更新坐标标签和边框
    updateCornerCoordinates();
    updateViewBorder();
    
    // 记录轨迹（每3帧记录一次）
    static int counter = 0;
    if (++counter % 3 == 0) {
        trajectory.append(carPosition);
        if (trajectory.size() > 200) trajectory.removeFirst(); // 限制轨迹点数
        drawTrajectory();
    }
    
    // 更新状态显示
    updateStatusDisplay();
}

void MainWindow::drawTrajectory()
{
    // 清除全部旧轨迹
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem *item : items) {
        if (item->data(0).toString() == "trajectory" || 
            item->data(0).toString() == "figure8_path") {
            scene->removeItem(item);
            delete item;
        }
    }
    
    // 绘制新轨迹
    QPen pen;
    if(driveMode) {
        pen.setColor(Qt::gray);  // 自动轨迹
    } else {
        pen.setColor(Qt::green);  // 手动轨迹
    }
    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    
    // 小车200个点的已移动轨迹
    for (int i = 1; i < trajectory.size(); i++) {
        QGraphicsLineItem *line = scene->addLine(
            trajectory[i-1].x(), trajectory[i-1].y(),
            trajectory[i].x(), trajectory[i].y(), pen
        );
        line->setData(0, "trajectory"); // 标记为轨迹
        line->setZValue(-1); // 置于底层
    }
    
    // 在自动模式下，绘制完整的规划路径
    if(driveMode) {
        QPainterPath path;
        if(!figurePoints.isEmpty()) {
            path.moveTo(figurePoints[0]);
            for(int i = 1; i < figurePoints.size(); i++) {
                path.lineTo(figurePoints[i]);
            }
            path.closeSubpath(); // 闭合路径
            
            QGraphicsPathItem *figure8Path = new QGraphicsPathItem(path);
            // 规划路径使用淡灰色（半透明）
            figure8Path->setPen(QPen(QColor(200, 200, 200, 150), 1)); 
            figure8Path->setBrush(Qt::NoBrush);
            figure8Path->setZValue(-2); // 在轨迹之下
            scene->addItem(figure8Path);
            figure8Path->setData(0, "figure8_path");
        }
    }
}

void MainWindow::updateStatusDisplay()
{
    QString modeText = driveMode ? "自动模式" : "手动模式";
    
    // 显示状态信息
    QString status = QString("模式: %6\n位置: (%1, %2)\n"
                             "方向: %3°\n"
                             "速度: %4 像素/帧\n"
                             "轨迹点: %5")
                    .arg(carPosition.x(), 0, 'f', 1)
                    .arg(carPosition.y(), 0, 'f', 1)
                    .arg(carDirection, 0, 'f', 1)
                    .arg(carSpeed, 0, 'f', 1)
                    .arg(trajectory.size())
                    .arg(modeText);
    
    ui->statusLabel->setText(status);
}

void MainWindow::updateCornerCoordinates()
{
    // 获取当前视图范围
    QRectF viewRect = ui->graphicsView->mapToScene(
        ui->graphicsView->viewport()->rect()
    ).boundingRect();
    
    // 获取视图的四个角点
    QPointF topLeft = viewRect.topLeft();
    QPointF topRight = viewRect.topRight();
    QPointF bottomLeft = viewRect.bottomLeft();
    QPointF bottomRight = viewRect.bottomRight();
    
    // 设置标签内容（格式化为整数）
    cornerLabels[0]->setText(QString("(%1, %2)").arg(topLeft.x(), 0, 'f', 0).arg(topLeft.y(), 0, 'f', 0));
    cornerLabels[1]->setText(QString("(%1, %2)").arg(topRight.x(), 0, 'f', 0).arg(topRight.y(), 0, 'f', 0));
    cornerLabels[2]->setText(QString("(%1, %2)").arg(bottomLeft.x(), 0, 'f', 0).arg(bottomLeft.y(), 0, 'f', 0));
    cornerLabels[3]->setText(QString("(%1, %2)").arg(bottomRight.x(), 0, 'f', 0).arg(bottomRight.y(), 0, 'f', 0));
    
    // 设置标签位置（考虑到标签宽度，进行偏移）
    const int padding = 5;
    
    // 左上角标签（放置在左上角内部）
    cornerLabels[0]->setPos(topLeft + QPointF(padding, padding));
    
    // 右上角标签（放置在右上角内部）
    cornerLabels[1]->setPos(topRight.x() - cornerLabels[1]->boundingRect().width() - padding, 
                           topRight.y() + padding);
    
    // 左下角标签（放置在左下角内部）
    cornerLabels[2]->setPos(bottomLeft.x() + padding,
                           bottomLeft.y() - cornerLabels[2]->boundingRect().height() - padding);
    
    // 右下角标签（放置在右下角内部）
    cornerLabels[3]->setPos(bottomRight.x() - cornerLabels[3]->boundingRect().width() - padding,
                           bottomRight.y() - cornerLabels[3]->boundingRect().height() - padding);
}

void MainWindow::updateViewBorder()
{
    // 获取当前视图范围
    QRectF viewRect = ui->graphicsView->mapToScene(
        ui->graphicsView->viewport()->rect()
    ).boundingRect();
    
    // 更新视图边框
    viewBorder->setRect(viewRect);
}

void MainWindow::centerViewOnCar()
{
    // 设置视图中心为小车位置
    ui->graphicsView->centerOn(carPosition);
}

void MainWindow::loadImage()
{
    QString filename = QFileDialog::getOpenFileName(
            this, 
            "选择图片", 
            QDir::homePath(), 
            "Images (*.png *.jpg *.bmp)"
        );
        
    if (filename.isEmpty()) return;
    
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    
    if (image.empty()) {
        QMessageBox::critical(this, "Error", "图片加载失败！");
        return;
    }
    
    // 处理图像并提取曲线点
    extractCurvePoints(image);
    
    // 在场景中显示结果
    // scene->clear();
    displayPoints();
    adjustFigure();
}

void MainWindow::extractCurvePoints(cv::Mat image) 
{
    figurePoints.clear();

    // 1. 二值化图像
    cv::Mat binary;
    cv::threshold(image, binary, 128, 255, cv::THRESH_BINARY_INV);

    // 2. 提取图像骨架（中心线）
    cv::Mat skeleton = cv::Mat::zeros(binary.size(), CV_8UC1);
    cv::Mat temp;
    cv::Mat eroded;

    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

    bool done;
    do {
        cv::erode(binary, eroded, element);
        cv::dilate(eroded, temp, element);
        cv::subtract(binary, temp, temp);
        cv::bitwise_or(skeleton, temp, skeleton);
        eroded.copyTo(binary);
        
        done = (cv::countNonZero(binary) == 0);
    } while (!done);

    // 3. 查找骨架轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(skeleton, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    // 4. 找到最长的轮廓
    int maxIndex = -1;
    double maxLength = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        double length = cv::arcLength(contours[i], false);
        if (length > maxLength) {
            maxLength = length;
            maxIndex = i;
        }
    }

    // 5. 对轮廓点进行平滑处理
    if (maxIndex >= 0) {
        std::vector<cv::Point> smoothedContour;
        
        // 使用高斯滤波平滑轮廓
        std::vector<cv::Point2f> contourFloat;
        for (const auto& pt : contours[maxIndex]) {
            contourFloat.emplace_back(pt.x, pt.y);
        }
        
        // 应用高斯平滑
        cv::Mat contourMat(contourFloat);
        cv::GaussianBlur(contourMat, contourMat, cv::Size(5, 5), 1.5);
        
        // 转换为整数点
        for (int i = 0; i < contourMat.rows; i++) {
            cv::Point2f pt = contourMat.at<cv::Point2f>(i);
            smoothedContour.emplace_back(cvRound(pt.x), cvRound(pt.y));
        }
        
        // 6. 采样点以减少点数并保持平滑
        const int sampleStep = 5; // 每5个点采样一个
        for (int i = 0; i < smoothedContour.size(); i += sampleStep) {
            figurePoints.append(QPointF(smoothedContour[i].x, smoothedContour[i].y));
        }
    }
}

void MainWindow::displayPoints() {
    if (figurePoints.isEmpty()) return;

    scene_2->clear();
    
    // 绘制曲线
    QPainterPath path;
    path.moveTo(figurePoints.first());
    
    for (int i = 1; i < figurePoints.size(); i++) {
        path.lineTo(figurePoints[i]);
    }
    
    auto pathItem = new QGraphicsPathItem(path);
    pathItem->setPen(QPen(Qt::red, 1));
    scene_2->addItem(pathItem);

    // 自适应视图范围（带边距）
    QRectF pathRect = path.boundingRect().adjusted(-20, -20, 20, 20); // 增加20px边距
    scene_2->setSceneRect(pathRect);
    ui->graphicsView_2->fitInView(pathRect, Qt::KeepAspectRatio);
}