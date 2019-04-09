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
  <body>
  Lorem Ipsum<input width="50px" height="50px" type="checkbox" name="nameOfChoice" value="1" style="width:50px;height:50px;background:darkgrey;">

  <customtag>customtag</customtag>
  <link rel="stylesheet" href="://res/css/reset.css">
  <link rel="stylesheet" href="://res/css/bootstrap.css">
  <link rel="stylesheet" href="://res/css/test1.css">

  <div id='home'>dddd</div>

  <div style='margin:15px;font: "Droid Sans";background:#CCC;border-bottom:6px solid yellow;width:30px;height:30px'>РУСtext1</div>
  <div style='font: "Arial Unicode MS";background:cyan;border-right:6px solid yellow;width:30px;height:30px'>РУСtext2</div>
  <div style='font: "Cousine Regular";background:#EEE;border:3px solid lightblue;width:30px;height:30px'>РУСtext3</div>
  <div style='font: "Font Awesome 5 Free";background:pink;border-left:16px solid grey;width:30px;height:30px'>
  )raw";

  htmlStr += "\uf15c ? ";
  htmlStr += u8"\uf15c";
  htmlStr += u8"\uf118";
  htmlStr += u8"\uf118 ? \uf118";


  htmlStr += R"raw(\uf118 ? \uf118</div>

  <ul><li>Oneвапвап</li><li>Zwei\u6211\u662F\u4E2D\u6587</li><li>Trois</li></ul>
  <bold>123SFSDFDFSDF</bold>
  <strong>emphasized text</strong>
  <span style='font-weight: bold;'>bold text</span>
  <p>Line1.1 Line1.2<br />Line2</p><ul><li>One</li><li>Zwei</li><li>Trois</li></ul>
  <table><tr><th>H1</th><th>H2</th></tr><tr><td>C1.1</td><td>C1.2</td></tr><tr><td>C2.1</td><td>C2.2</td></tr></table>
  <img src=':/res/img/test.png' alt=''>
  <img src=':/res/img/test2.png' alt=''>




<ul class="pagination center">
<li class="page-item disabled">
    <a class="page-link" href="#" title="Back"><i class="material-icons keyboard_arrow_left">keyboard_arrow_left</i></a>
</li>
<li class="page-item">
  <a class="page-link" href="#">1</a>
</li>
<li class="page-item active">
    <a class="page-link" href="#">2 <span class="sr-only">(current)</span></a>
</li>
<li class="page-item">
  <a class="page-link" href="#">3</a>
</li>
<li class="page-item">
    <a class="page-link" href="#" title="Next"><i class="material-icons keyboard_arrow_right">keyboard_arrow_right</i></a>
</li>
</ul>

<br>
dsasdasad
<div class="pager center">
    <a class="page-link disabled" href="#" title="Back"><i class="material-icons keyboard_arrow_left">keyboard_arrow_left</i><span>Previous Article</span></a>
    <a class="page-link" href="#" title="Next"><span>ghjhjh Article</span><i class="material-icons keyboard_arrow_right">keyboard_arrow_right</i></a>
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
