#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QListWidget>
#include <QtableWidget>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QRandomGenerator>
#include <QFile>
#include <QTextStream>
#include <cmath>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 初始化成员变量
    m_dataCount = 30;
    m_rangeX = 0.01;
    m_rangeY = 0.01;
    m_precision = 0.02;
    m_cpkX = 0.0;
    m_cpkY = 0.0;
    m_averageX = 0.0;
    m_averageY = 0.0;
    m_stdDevX = 0.0;
    m_stdDevY = 0.0;

    ui->setupUi(this);
    // 连接信号槽（如果自动连接失败）
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::on_pushButton_4_clicked);
    connect(ui->lineEdit_y, &QLineEdit::textEdited, this, &MainWindow::on_lineEdit_y_textEdited);
    connect(ui->lineEdit_count, &QLineEdit::textEdited, this, &MainWindow::on_lineEdit_count_textEdited);
    connect(ui->lineEdit_4, &QLineEdit::textEdited, this, &MainWindow::on_lineEdit_4_textEdited);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// UI初始化函数
void MainWindow::SetUi()
{
    // 设置表格属性
    QStringList header;
    header << "x 偏差" << "y 偏差";
    ui->tableWidget->setColumnCount(header.size());
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 不可编辑
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 列宽自动拉伸

    // 设置输入框初始值
    QString strx = QString::number(m_rangeX);
    QString stry = QString::number(m_rangeY);
    ui->lineEdit_x->setText(strx);
    ui->lineEdit_y->setText(stry);

    strx = QString::number(m_dataCount);
    ui->lineEdit_count->setText(strx);

    strx = QString::number(m_precision);
    ui->lineEdit_4->setText(strx);

    // 设置结果显示框
    stry = QString::number(m_cpkX, 'f', 4);
    ui->lineEdit_count_2->setText(stry);

    stry = QString::number(m_cpkY, 'f', 4);
    ui->lineEdit_5->setText(stry);

    stry = QString::number(m_averageX, 'f', 4);
    ui->lineEdit_count_3->setText(stry);

    stry = QString::number(m_averageY, 'f', 4);
    ui->lineEdit_6->setText(stry);
}
// 生成随机数据
void MainWindow::get_date()
{
    m_dataPoints.clear();
    PointData m_point;
    
    for(int i = 0; i < m_dataCount; i++)
    {
        // 生成-范围到+范围的随机浮点数，使数据更加真实
        double randomInRangeX = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * m_rangeX;
        double randomInRangeY = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * m_rangeY;
        
        m_point.piancha_x = randomInRangeX;
        m_point.piancha_y = randomInRangeY;
        m_point.iDex = i + 1;
        m_point.StrDex = QString::number(m_point.iDex);
        m_dataPoints.push_back(m_point);
    }
}

// 显示数据到表格
void MainWindow::generate_date()
{
    // 删除表中所有数据
    int row = ui->tableWidget->rowCount();
    for (int i = 0; i < row; i++)
        ui->tableWidget->removeRow(0);

    // 设置表格行数
    ui->tableWidget->setRowCount(m_dataPoints.size());
    
    // 填充表格数据
    for (int row = 0; row < m_dataPoints.size(); row++)
    {
        QString strx = QString::number(m_dataPoints[row].piancha_x, 'f', 6);
        QString stry = QString::number(m_dataPoints[row].piancha_y, 'f', 6);
        
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(strx));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(stry));
    }
}

// 生成随机数据按钮点击事件
void MainWindow::on_pushButton_clicked()
{
    get_date();
    generate_date();
    // 自动计算CPK值
    calculateCPK();
}

// x范围输入框文本变化事件
void MainWindow::on_lineEdit_x_textEdited(const QString &arg1)
{
    double val = arg1.toDouble();
    if (val > 0)
        m_rangeX = val;
}

// y范围输入框文本变化事件
void MainWindow::on_lineEdit_y_textEdited(const QString &arg1)
{
    double val = arg1.toDouble();
    if (val > 0)
        m_rangeY = val;
}

// 数据数量输入框文本变化事件
void MainWindow::on_lineEdit_count_textEdited(const QString &arg1)
{
    int val = arg1.toInt();
    if (val > 0)
        m_dataCount = val;
}

// 精度输入框文本变化事件
void MainWindow::on_lineEdit_4_textEdited(const QString &arg1)
{
    double val = arg1.toDouble();
    if (val > 0)
        m_precision = val;
}

// 清除数据按钮点击事件
void MainWindow::on_pushButton_3_clicked()
{
    // 删除表中所有数据
    int row = ui->tableWidget->rowCount();
    for (int i = 0; i < row; i++)
        ui->tableWidget->removeRow(0);
    
    // 清空数据集合
    m_dataPoints.clear();
    
    // 重置计算结果
    m_cpkX = 0.0;
    m_cpkY = 0.0;
    m_averageX = 0.0;
    m_averageY = 0.0;
    m_stdDevX = 0.0;
    m_stdDevY = 0.0;
    
    // 更新显示
    ui->lineEdit_count_2->setText("0.0000");
    ui->lineEdit_5->setText("0.0000");
    ui->lineEdit_count_3->setText("0.0000");
    ui->lineEdit_6->setText("0.0000");
}

// 打开文件对话框
void MainWindow::FileOpen()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("导入数据文件"));
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("数据文件 (*.csv *.txt *.dat)"));
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList files = dialog.selectedFiles();
        if (!files.isEmpty())
        {
            QString fileName = files.first();
            importDataFromFile(fileName);
        }
    }
}

// 从文件导入数据
void MainWindow::importDataFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件: %1").arg(fileName));
        return;
    }

    QTextStream in(&file);
    m_dataPoints.clear();
    int lineNumber = 0;
    
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        
        // 尝试用逗号或制表符分割
        QStringList parts = line.split(",", Qt::SkipEmptyParts);
        if (parts.isEmpty())
            parts = line.split("\t", Qt::SkipEmptyParts);
        
        if (parts.size() >= 2)
        {
            PointData point;
            point.iDex = ++lineNumber;
            point.StrDex = QString::number(point.iDex);
            
            // 尝试转换为数值
            bool okX, okY;
            double x = parts[0].toDouble(&okX);
            double y = parts[1].toDouble(&okY);
            
            if (okX && okY)
            {
                point.piancha_x = x;
                point.piancha_y = y;
                m_dataPoints.push_back(point);
            }
        }
    }
    
    file.close();
    
    // 更新数据数量
    m_dataCount = m_dataPoints.size();
    ui->lineEdit_count->setText(QString::number(m_dataCount));
    
    // 显示数据并计算CPK
    generate_date();
    calculateCPK();
    
    QMessageBox::information(this, tr("成功"), tr("数据导入成功，共导入 %1 条数据").arg(m_dataCount));
}

// 导入数据按钮点击事件
void MainWindow::on_pushButton_2_clicked()
{
    FileOpen();
}

// 计算CPK按钮点击事件
void MainWindow::on_pushButton_4_clicked()
{
    calculateCPK();
}

// 计算平均值
double MainWindow::calculateAverage(const std::vector<double> &values)
{
    if (values.empty())
        return 0.0;
    
    double sum = 0.0;
    for (double value : values)
        sum += value;
    
    return sum / values.size();
}

// 计算标准差
double MainWindow::calculateStandardDeviation(const std::vector<double> &values, double average)
{
    if (values.size() <= 1)
        return 0.0;
    
    double sum = 0.0;
    for (double value : values)
        sum += pow(value - average, 2);
    
    return sqrt(sum / (values.size() - 1));
}

// 计算CPK值
void MainWindow::calculateCPK()
{
    if (m_dataPoints.empty())
    {
        QMessageBox::warning(this, tr("警告"), tr("没有数据可以计算CPK值"));
        return;
    }
    
    // 提取x和y的值
    std::vector<double> xValues, yValues;
    for (const auto &point : m_dataPoints)
    {
        xValues.push_back(point.piancha_x);
        yValues.push_back(point.piancha_y);
    }
    
    // 计算平均值
    m_averageX = calculateAverage(xValues);
    m_averageY = calculateAverage(yValues);
    
    // 计算标准差
    m_stdDevX = calculateStandardDeviation(xValues, m_averageX);
    m_stdDevY = calculateStandardDeviation(yValues, m_averageY);
    
    // 计算CPK值
    // CPK = min[(USL - μ)/(3σ), (μ - LSL)/(3σ)]
    // 这里使用精度作为规格限
    double specLimitX = m_precision;
    double specLimitY = m_precision;
    
    if (m_stdDevX > 0)
    {
        double cpkUslX = (specLimitX - m_averageX) / (3 * m_stdDevX);
        double cpkLslX = (m_averageX + specLimitX) / (3 * m_stdDevX);
        m_cpkX = std::min(cpkUslX, cpkLslX);
    }
    else
    {
        m_cpkX = 0.0;
    }
    
    if (m_stdDevY > 0)
    {
        double cpkUslY = (specLimitY - m_averageY) / (3 * m_stdDevY);
        double cpkLslY = (m_averageY + specLimitY) / (3 * m_stdDevY);
        m_cpkY = std::min(cpkUslY, cpkLslY);
    }
    else
    {
        m_cpkY = 0.0;
    }
    
    // 更新显示
    ui->lineEdit_count_2->setText(QString::number(m_cpkX, 'f', 4));
    ui->lineEdit_5->setText(QString::number(m_cpkY, 'f', 4));
    ui->lineEdit_count_3->setText(QString::number(m_averageX, 'f', 4));
    ui->lineEdit_6->setText(QString::number(m_averageY, 'f', 4));
    
    // 根据CPK值提供建议
    QString suggestion;
    if (m_cpkX >= 1.67 && m_cpkY >= 1.67)
        suggestion = tr("CPK值良好，考虑降低成本");
    else if ((m_cpkX >= 1.33 && m_cpkX < 1.67) && (m_cpkY >= 1.33 && m_cpkY < 1.67))
        suggestion = tr("状态良好，维持现状");
    else if ((m_cpkX >= 0.67 && m_cpkX < 1.0) || (m_cpkY >= 0.67 && m_cpkY < 1.0))
        suggestion = tr("制程不良较多，必须提升能力改进");
    else if (m_cpkX < 0.67 || m_cpkY < 0.67)
        suggestion = tr("支撑能力较差，考虑整体设计制程");
    
    if (!suggestion.isEmpty())
        statusBar()->showMessage(suggestion, 5000); // 显示5秒
}

