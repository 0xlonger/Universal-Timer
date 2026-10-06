#include "ui/WelcomePage.h"
#include "core/ThemeManager.h"

WelcomePageClass::WelcomePageClass(QWidget* parent)
    : QWidget(parent)
{
    WelcomeLabel = new QLabel(tr("欢迎使用万能倒计时!<br>简单设置，并开始使用吧!"), this);
    WelcomeLabel->setAlignment(Qt::AlignCenter);
    WelcomeLabel->resize(this->size());
    WelcomeLabel->setStyleSheet(ThemeManager::instance().style("WelcomeLabel"));
    WelcomeLabel->show();

    WelcomeButton = new QPushButton(tr("开始"), this);
    WelcomeButton->setGeometry(this->width() * 0.45, this->height() * 0.6, this->width() * 0.1, this->height() * 0.1);
    WelcomeButton->setStyleSheet(ThemeManager::instance().style("WelcomeButton"));
    WelcomeButton->show();
    
    connect(WelcomeButton, &QPushButton::clicked, [this] {
        emit finished();
        });
}

WelcomePageClass::~WelcomePageClass()
{}

void WelcomePageClass::resizeEvent(QResizeEvent* event) {
    WelcomeLabel->resize(this->size());
    WelcomeButton->setGeometry(this->width() * 0.45, this->height() * 0.6, this->width() * 0.1, this->height() * 0.1);
}