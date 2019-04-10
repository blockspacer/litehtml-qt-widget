// -*- coding: utf-8 -*-

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "container_qt5.h"

#include <QDebug>
#include <QFile>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_litehtmlWidget = new litehtmlWidget();

  //QFile master_css_fh("/home/snoopy/projects/litehtml-qt/include/master.css");
  QFile master_css_fh("://res/css/master.css");
  master_css_fh.open(QIODevice::ReadOnly);
  QByteArray master_css = master_css_fh.readAll();
  //qDebug() << master_css;
  ctxt.load_master_stylesheet(master_css.constData());

  std::string htmlStr = "";
  htmlStr += R"raw(
  <html>
  <head>
  <link rel="stylesheet" href="://res/css/reset.css">
  <link rel="stylesheet" href="://res/css/test1.css">
  </head>
  <body>

<style>
#root {
    width: 500px;
    display: inline;
}
.first {
    background-color: #ddd;
    padding: 10px;
    color: black;
    margin-bottom: 5px;
    border-bottom: 1px solid red;
}
.first div {
    background-color: white;
    border: 1px solid black;
    height: 50px;
    line-height: 100%;
    text-align: center;
    vertical-align: middle;
}
#root.second .first div {
    border-color: red;
    background-color: #aaa;
    color: white;
}
.first .bunt {
    border-radius: 25px;
    border-top: 5px solid silver;
    border-right: 5px solid blue;
    border-bottom: 5px solid black;
    border-left: 5px solid red;
}
.first .bunt2 {
    border-radius: 5px;
    border-top: 5px solid red;
    border-right: 1px solid blue;
    border-left: 1px solid blue;
    border-bottom: 1px solid blue;
}
</style>

<div id="root">
  <div class="first">
    <div style="">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px">5px width / 0px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 1px;">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 5px;">5px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 5px;">1px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 15px;">5px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 15px;">5px 15px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 25px;">5px 15px width / 25px radius</div>
  </div>
  <div class="first">
    <div class="bunt">Bunt</div>
  </div>
</div>
<div id="root" class="second">
  <div class="first">
    <div style="">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px">5px width / 0px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 1px;">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 5px;">5px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 5px;">1px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 15px;">5px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 15px;">5px 15px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 25px;">5px 15px width / 25px radius</div>
  </div>
  <div class="first">
    <div class="bunt2">Bunt 2</div>
  </div>
</div>

  </body>
  </html>)raw";

  const char* html = htmlStr.c_str();

  //QString html2 = QString::fromUtf8(qPrintable(u8"F?llungRaupeStepываываtext"));//tr("F?llungRaupeStepываываtext");


  //auto doc = litehtml::document::createFromUTF8("<html><body><ul><li>One</li><li>Zwei</li><li>Trois</li></ul></body></html>", &c, &ctxt);
  //auto doc = litehtml::document::createFromUTF8("<html><body><p>Line1.1 Line1.2<br />Line2</p><ul><li>One</li><li>Zwei</li><li>Trois</li></ul></body></html>", &c, &ctxt);
  //auto doc = litehtml::document::createFromUTF8("<html><body><a href=\"http://linuxfr.org/\">DLFP</a></body></html>", &c, &ctxt);
  //auto doc = litehtml::document::createFromUTF8("<html><body><table><tr><th>H1</th><th>H2</th></tr><tr><td>C1.1</td><td>C1.2</td></tr><tr><td>C2.1</td><td>C2.2</td></tr></table></body></html>", c, &ctxt);
  auto doc = litehtml::document::createFromUTF8(qPrintable(html), m_litehtmlWidget->getContainer(), &ctxt);
  m_litehtmlWidget->getContainer()->set_document(doc);

  m_litehtmlWidget->show();

  ui->scrollAreaVerticalLayout->addWidget(m_litehtmlWidget);

  m_litehtmlWidget->repaint();

  //c->setGeometry(0, 0, 500, 1200);

  /*//ui->scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  //ui->scrollArea->setGeometry(0,0,400, 400);
  ui->scrollArea->verticalScrollBar()->setMinimum(0);
  ui->scrollArea->verticalScrollBar()->setMaximum(1500);//maximum();
  ui->scrollArea->verticalScrollBar()->setValue(50);//maximum();
  ui->scrollArea->verticalScrollBar()->setRange(0, 1500);
  //ui->scrollArea->verticalScrollBar()->setFixedSize(10, 10);
  ui->scrollArea->setEnabled(true);
  ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ui->scrollArea->resize(500,500);
  ui->scrollArea->setWidgetResizable(true);*/

  /*auto slider = new QSlider(Qt::Vertical);
  slider->setFocusPolicy(Qt::StrongFocus);
  slider->setTickPosition(QSlider::TicksBothSides);
  slider->setTickInterval(10);
  slider->setSingleStep(1);
  connect(slider, &QSlider::valueChanged, slider, [](int val){
    qDebug() << val;
  });

  ui->scrollAreaVerticalLayout->addWidget(slider);*/

  QScrollBar* scrollBar = new QScrollBar(Qt::Vertical);
  scrollBar->setParent(this);
  scrollBar->setFocusPolicy(Qt::StrongFocus);
  scrollBar->setMinimum(0);
  scrollBar->setMaximum(0);
  scrollBar->setValue(0);
  scrollBar->setSingleStep(1);
  scrollBar->setInvertedControls(false);
  connect(scrollBar, &QScrollBar::valueChanged, scrollBar, [this](int val){
    //qDebug() << "QScrollBar::valueChanged " << val;
    m_litehtmlWidget->getContainer()->setScrollY(-val);
    m_litehtmlWidget->repaint();
  });

  connect(m_litehtmlWidget->getContainer(), &container_qt5::docSizeChanged, m_litehtmlWidget->getContainer(), [this, scrollBar](int w, int h){
    //qDebug() << "docSizeChanged " << w << h;
    if (!scrollBar) {
      return;
    }

    const int singleVisiblePageHeight = m_litehtmlWidget->height();

    ///\note substracts already visible area = widget size
    const int alailibleScroll = h - singleVisiblePageHeight;
    scrollBar->setMaximum(alailibleScroll);
    // amount that the value changes on cursor keys
    scrollBar->setSingleStep(singleVisiblePageHeight);
    // amount that the value changes on Page Up and Page Down keys
    scrollBar->setPageStep(singleVisiblePageHeight);
  });

  ui->verticalLayoutBar->addWidget(scrollBar);
}

MainWindow::~MainWindow()
{
delete ui;
}
