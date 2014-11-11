#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *main_window;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    emu_thread = &emu;

    ui->setupUi(this);

    connect(&refresh_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    connect(&emu, SIGNAL(serialChar(char)), this, SLOT(serialChar(char)), Qt::QueuedConnection);
    connect(ui->actionDebugger, SIGNAL(triggered()), &emu, SLOT(enterDebugger()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(debugCommand()));

    refresh_timer.setInterval(1000 / 60); //60 fps
    refresh_timer.start();

    ui->lcdView->setScene(&lcd_scene);

    emu.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

extern "C"
{
    #include "lcd.h"
}

void MainWindow::refresh()
{
    lcd_scene.clear();

    if(emulate_cx)
    {
        QByteArray framebuffer(320 * 240 * 2, 0);
        uint32_t bitfields[3];
        lcd_cx_draw_frame(reinterpret_cast<uint16_t*>(framebuffer.data()), bitfields);
        QImage::Format format = bitfields[0] == 0x00F ? QImage::Format_RGB444 : QImage::Format_RGB16;
        QImage image(reinterpret_cast<const uchar*>(framebuffer.data()), 320, 240, 320 * 2, format);

        lcd_scene.addPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::serialChar(char c)
{
    if(c == '\r')
        return;

    QString s(c);

    ui->serialConsole->moveCursor(QTextCursor::End);
    ui->serialConsole->insertPlainText(s);
}

void MainWindow::debugCommand()
{
    debug_command = ui->lineEdit->text().toLatin1();
    emit debuggerCommand();
}