#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QVector>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include "global.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    
private slots:
    // 控制按钮槽函数
    void onLeftPressed();
    void onRightPressed();
    void onAccelPressed();
    void onDecelPressed();
    void onBrakePressed();
    void onFigure8Pressed(); // 8字形轨迹按钮
    void onFigureHandWritePressed();
    
    // 运动更新
    void updateCarPosition();
    
    // 控制按钮释放
    void releaseControls();

    void loadImage();
    void onInitPressed();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene, *scene_2;
    
    // 小车属性
    QGraphicsItemGroup *carGroup;  // 小车组包含车身和车头指示器
    QGraphicsRectItem *carBody;     // 小车车身
    QGraphicsPolygonItem *carHead; // 车头指示器
    QVector<QPointF> trajectory;
    const double CAR_LENGTH = 60.0; // 小车长度（像素）
    const double CAR_WIDTH = 30.0;  // 小车宽度
    
    // 运动参数
    double carSpeed = 0;   // 像素/帧
    double carDirection = 0; // 角度（初始0°）
    QPointF carPosition;  // 中心点坐标
    
    // 控制状态
    bool leftPressed = false;
    bool rightPressed = false;
    bool accelPressed = false;
    bool decelPressed = false;
    int driveMode = manualMode; // 自动模式标志
    
    // 轨迹参数
    QVector<QPointF> figurePoints;
    int figureIndex = 0; // 当前8字形点索引
    double figure8Size = 300; // 8字形大小
    
    // 定时器
    QTimer *timer;
    
    // 坐标标注
    QGraphicsSimpleTextItem *cornerLabels[4]; // 四个角的坐标标签
    
    // 视图边框
    QGraphicsRectItem *viewBorder, *viewBorder_2;
    
    // 更新状态显示
    void updateStatusDisplay();
    
    // 绘制轨迹
    void drawTrajectory();
    
    // 更新角落坐标标签
    void updateCornerCoordinates();

    void updateSceneRect();
    
    // 确保视图中心跟随小车
    void centerViewOnCar();
    
    // 创建车头指示器
    void createCarHeadIndicator();
    
    // 生成8字形轨迹点（以当前位置为起点，沿当前方向）
    void generateFigure8();
    
    // 更新视图边框
    void updateViewBorder();

    void extractCurvePoints(cv::Mat image);
    void displayPoints();
    void adjustFigure();
};
#endif // MAINWINDOW_H
