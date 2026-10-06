#include "ui/AboutPage.h"
#include "core/Global.h"

#include <QGridLayout>

AboutPageClass::AboutPageClass(QWidget* parent)
    : QWidget(parent)
{

    this->setWindowTitle(tr("关于"));

    IconLabel = new ScalableLabel(this);
    IconLabel->setPixmap(QPixmap(":/images/icons/Universal-Timer-2_icon.512px.png"));
    IconLabel->show();

    AboutLabel = new QLabel(
        tr("万能倒计时<br>"
            "版本：%1<br>"
            "开发者：<br>"
            "    龙ger_longer (B站同名，GitHub 用户名 0xlonger)<br>"
            "    new_pointer (B站同名，GitHub 用户名 new5Fpointer)<br>"
            "    XUESHENG_XSH (B站同名，GitHub 用户名 bilixuesheng)<br>"
            ).arg(CURRENT_VERSION_STRING),
        this
        );
    AboutLabel->setWordWrap(true);
    AboutLabel->setAlignment(Qt::AlignLeft);
    AboutLabel->show();

    AboutTextEdit = new QTextEdit(
        tr("欢迎使用万能倒计时！<br>"
            "本软件代码使用 GPLv3 许可证，如果您是开发者，请遵守许可证。<br>"
            "本软件完全免费且开源，赞助完全取决于自愿！<br>"
            "您可以通过托盘图标进入设置界面，根据您的喜好个性化本软件。<br>"
            "截至该版本发布，仍暂时没有自动更新的功能。由此查看万能倒计时的各个版本：%1<br>"
            "网站（可能会变动，请注意关注最新动向）：<br>"
            "&nbsp;&nbsp;&nbsp;&nbsp;%2<br>"
            "&nbsp;&nbsp;&nbsp;&nbsp;若无法访问，请前往：%3 或 %4"
            ).arg(GITHUB_RELEASE_URL).arg(DOMAIN_URL).arg(GITHUB_PAGES_DOMAIN_URL).arg(CLOUDFLARE_PAGES_DOMAIN_URL),
        this
        );
    AboutTextEdit->setReadOnly(true);
    AboutLabel->show();

    QGridLayout* Layout = new QGridLayout(this);
    Layout->addWidget(IconLabel, 0, 0, 1, 1);
    Layout->addWidget(AboutLabel, 0, 1, 1, 1);
    Layout->addWidget(AboutTextEdit, 1, 0, 1, 2);
    Layout->setSpacing(25);
    this->setLayout(Layout);

}

AboutPageClass::~AboutPageClass()
{}
