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

    // 初始设置视图中心为(0,0)
    ui->graphicsView->centerOn(0, 0);
    
    // 初始更新坐标标签和边框
    updateCornerCoordinates();
    updateViewBorder();
    
    // 设置初始场景范围（以小车为中心）
    updateSceneRect();
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
    figureAutoMode = manualMode;
}

void MainWindow::onRightPressed()
{
    rightPressed = true;
    figureAutoMode = manualMode;
}

void MainWindow::onAccelPressed()
{
    accelPressed = true;
    figureAutoMode = manualMode;
}

void MainWindow::onDecelPressed()
{
    decelPressed = true;
    figureAutoMode = manualMode;
}

void MainWindow::onBrakePressed()
{
    carSpeed = 0;  // 急刹停车
    figureAutoMode = manualMode;
}

void MainWindow::onFigure8Pressed()
{
    figureAutoMode = figure8Mode; // 切换8字形模式
    
    if(figureAutoMode) {
        // 进入8字形模式
        // 以当前小车位置为起点，当前方向为起始方向生成8字形
        generateFigure8();
        
        figure8Index = 1;
        carSpeed = 5; // 设置固定速度

        // trajectory.append(carPosition); // 保留当前位置
        
        // 重绘轨迹
        // drawTrajectory();
    } else {
        // 退出8字形模式，小车停止
        carSpeed = 0;
    }
    
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
    figure8Points.clear();
    
    // 计算旋转角度（使曲线在起点处与x轴相切）
    double roAngle = -M_PI/4; // 旋转-45度（顺时针45度）

    // 标准8字形参数方程（以原点为中心）
    for(int i = 0; i < totalPoints; i++) {
        double t = 2.0 * M_PI * i / totalPoints;
        double x_orig = figure8Size * sin(t) / (1 + pow(cos(t), 2));
        double y_orig = figure8Size * sin(t) * cos(t) / (1 + pow(cos(t), 2));

        // 应用旋转（使曲线在t=0时与x轴相切）
        double x = x_orig * cos(roAngle) - y_orig * sin(roAngle);
        double y = x_orig * sin(roAngle) + y_orig * cos(roAngle);
        figure8Points.append(QPointF(x, y));
    }
    
    // 计算旋转角度（使8字形方向与小车当前方向一致）
    double rotationAngle = carDirection;
    
    // 创建变换矩阵
    QTransform transform;
    transform.translate(carPosition.x(), carPosition.y()); // 平移到小车当前位置
    transform.rotate(rotationAngle); // 旋转到小车当前方向
    
    // 应用变换到所有点
    for(int i = 0; i < figure8Points.size(); i++) {
        figure8Points[i] = transform.map(figure8Points[i]);
    }
}

void MainWindow::updateCarPosition()
{
    if(figureAutoMode) {
        // 8字形模式
        if(figure8Index < figure8Points.size()) {
            // 获取下一个8字形点
            QPointF nextPos = figure8Points[figure8Index];
            
            // 计算方向变化（基于当前位置和下一位置）
            double dx = nextPos.x() - carPosition.x();
            double dy = nextPos.y() - carPosition.y();
            carDirection = qRadiansToDegrees(std::atan2(dy, dx));
            
            // 更新位置
            carPosition = nextPos;
            figure8Index++;
            
            // 如果到达终点，回到起点
            if(figure8Index >= figure8Points.size()) {
                figure8Index = 0;
            }
        }
    } else {
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
    // 清除旧轨迹
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
    if(figureAutoMode) {
        pen.setColor(Qt::gray);  // 表示8字形轨迹
    } else {
        pen.setColor(Qt::green);  // 手动轨迹
    }
    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    
    for (int i = 1; i < trajectory.size(); i++) {
        QGraphicsLineItem *line = scene->addLine(
            trajectory[i-1].x(), trajectory[i-1].y(),
            trajectory[i].x(), trajectory[i].y(), pen
        );
        line->setData(0, "trajectory"); // 标记为轨迹
        line->setZValue(-1); // 置于底层
    }
    
    // 在8字形模式下，绘制完整的8字形路径（规划路径）
    if(figureAutoMode) {
        QPainterPath path;
        if(!figure8Points.isEmpty()) {
            path.moveTo(figure8Points[0]);
            for(int i = 1; i < figure8Points.size(); i++) {
                path.lineTo(figure8Points[i]);
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
    QString modeText = figureAutoMode ? "8字形模式" : "手动模式";
    
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