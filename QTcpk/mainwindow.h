#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 数据点结构体，用于存储x和y的偏差值
typedef struct Point_EX
{
    QString StrDex;  // 数据点索引（字符串形式）
    int     iDex;    // 数据点索引（整数形式）
    double  piancha_x; // x方向的偏差值
    double  piancha_y; // y方向的偏差值
} PointData;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void SetUi();            // UI初始化函数
    void get_date();         // 生成随机数据
    void generate_date();    // 显示数据到表格
    void FileOpen();         // 打开文件对话框
    void calculateCPK();     // 计算CPK值
    void importDataFromFile(const QString &fileName); // 从文件导入数据
    
    // 计算平均值
    double calculateAverage(const std::vector<double> &values);
    // 计算标准差
    double calculateStandardDeviation(const std::vector<double> &values, double average);
    
    int     m_dataCount;     // 数据数量
    double  m_rangeX;        // x的随机范围
    double  m_rangeY;        // y的随机范围
    double  m_precision;     // 精度
    
    // 数据存储
    std::vector<PointData> m_dataPoints;  // 存储所有数据点
    
    // 计算结果
    double  m_cpkX;          // x的CPK值
    double  m_cpkY;          // y的CPK值
    double  m_averageX;      // x的平均值
    double  m_averageY;      // y的平均值
    double  m_stdDevX;       // x的标准差
    double  m_stdDevY;       // y的标准差

private slots:
    void on_pushButton_clicked();       // 生成随机数据按钮点击
    void on_pushButton_2_clicked();     // 导入数据按钮点击
    void on_pushButton_3_clicked();     // 清除数据按钮点击
    void on_pushButton_4_clicked();     // 计算CPK按钮点击
    
    // 输入框内容变化槽函数
    void on_lineEdit_x_textEdited(const QString &arg1);
    void on_lineEdit_y_textEdited(const QString &arg1);
    void on_lineEdit_count_textEdited(const QString &arg1);
    void on_lineEdit_4_textEdited(const QString &arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
