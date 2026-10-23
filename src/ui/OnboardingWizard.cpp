#include "core/Global.h"
#include "ui/OnboardingWizard.h"
#include "ui/WelcomePage.h"
#include "ui/FloatingBar.h"
#include "core/ThemeManager.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QListView>

OnboardingWizardClass::OnboardingWizardClass(QWidget* parent, ConfigManager& cfg)
    : QWidget(parent), config(cfg)
{
    Pages = new QStackedWidget(this);

    // 欢迎页：点“开始”进入第一步
    WelcomePageClass* WelcomePage = new WelcomePageClass(Pages);
    connect(WelcomePage, &WelcomePageClass::finished, this, [this] { goTo(1); });
    Pages->addWidget(WelcomePage);

    // 语言
    QWidget* LanguageBody = new QWidget;
    QVBoxLayout* LanguageLayout = new QVBoxLayout(LanguageBody);
    LanguageLayout->setContentsMargins(0, 0, 0, 0);
    LanguageLabel = new QLabel(LanguageBody);
    LanguageLabel->setObjectName("OnboardingLanguage");
    QWidget* LanguageForm = new QWidget(LanguageBody);
    LanguageComboBox = new QComboBox(LanguageForm);
    for (const auto& [code, name] : AVAILABLE_LANGUAGES)
        LanguageComboBox->addItem(name, code);
    LanguageComboBox->setCurrentIndex(qMax(0, LanguageComboBox->findData(config.general.language)));
    LanguageLabel->setText(LanguageComboBox->currentText());
    QFormLayout* LanguageFormLayout = new QFormLayout(LanguageForm);
    LanguageFormLayout->setContentsMargins(0, 0, 0, 0);
    LanguageFormLayout->addRow(tr("语言："), LanguageComboBox);
    LanguageLayout->addWidget(LanguageLabel);
    LanguageLayout->addWidget(LanguageForm);
    LanguageLayout->addStretch();
    addStep(tr("这是正确的语言吗？"), tr("不对的话，在下面换一个。"), LanguageBody, { LanguageForm });

    // 主题
    QWidget* ThemeBody = new QWidget;
    QVBoxLayout* ThemeLayout = new QVBoxLayout(ThemeBody);
    ThemeLayout->setContentsMargins(0, 0, 0, 0);
    QWidget* ThemeForm = new QWidget(ThemeBody);
    ThemeComboBox = new QComboBox(ThemeForm);
    for (const ThemeManager::ThemeInfo& theme : ThemeManager::instance().themes())
        ThemeComboBox->addItem(theme.name, theme.id);
    ThemeComboBox->setCurrentIndex(qMax(0, ThemeComboBox->findData(ThemeManager::instance().currentId())));
    QFormLayout* ThemeFormLayout = new QFormLayout(ThemeForm);
    ThemeFormLayout->setContentsMargins(0, 0, 0, 0);
    ThemeFormLayout->addRow(tr("主题："), ThemeComboBox);
    ThemeLayout->addWidget(ThemeForm);
    ThemeLayout->addWidget(new QLabel(tr("悬浮条会显示成这样："), ThemeBody));
    ThemeLayout->addWidget(createPreview());
    ThemeLayout->addStretch();
    addStep(tr("选一个主题"), tr("主题决定悬浮条的样子，引导结束后悬浮条会出现在屏幕顶部。以后可以在 设置中心 → 总设置 → 主题 里换。"), ThemeBody, { ThemeForm });

    // 倒计时
    QWidget* CountdownBody = new QWidget;
    QVBoxLayout* CountdownLayout = new QVBoxLayout(CountdownBody);
    CountdownLayout->setContentsMargins(0, 0, 0, 0);
    QWidget* CountdownForm = new QWidget(CountdownBody);
    TargetDateTimeEdit = new QDateTimeEdit(config.general.target_date_time, CountdownForm);
    TargetDateTimeEdit->setCalendarPopup(true);
    TargetDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    FloatingBarTextLineEdit = new QLineEdit(config.floating_bar.floating_bar_text, CountdownForm);
    FloatingBarTextLineEdit->setPlaceholderText(tr("悬浮条文本"));
    QFormLayout* CountdownFormLayout = new QFormLayout(CountdownForm);
    CountdownFormLayout->setContentsMargins(0, 0, 0, 0);
    CountdownFormLayout->addRow(tr("目标时间："), TargetDateTimeEdit);
    CountdownFormLayout->addRow(tr("悬浮条文本："), FloatingBarTextLineEdit);
    CountdownLayout->addWidget(CountdownForm);
    CountdownLayout->addWidget(new QLabel(tr("悬浮条会显示成这样："), CountdownBody));
    CountdownLayout->addWidget(createPreview());
    CountdownLayout->addStretch();
    addStep(tr("倒计时到什么时候？"), tr("悬浮条会一直显示离这个时间还剩多久。"), CountdownBody, { CountdownForm });

    // 定时全屏提醒
    QWidget* ReminderBody = new QWidget;
    QVBoxLayout* ReminderLayout = new QVBoxLayout(ReminderBody);
    ReminderLayout->setContentsMargins(0, 0, 0, 0);
    IsShowReminderCheckBox = new QCheckBox(tr("显示全屏提醒"), ReminderBody);
    IsShowReminderCheckBox->setChecked(config.reminder.is_show_reminder);
    ReminderSettingsWidget = new QWidget(ReminderBody);
    QVBoxLayout* ReminderSettingsLayout = new QVBoxLayout(ReminderSettingsWidget);
    ReminderSettingsLayout->setContentsMargins(0, 0, 0, 0);
    QWidget* ReminderForm = new QWidget(ReminderSettingsWidget);
    ReminderTitleLineEdit = new QLineEdit(config.reminder.reminder_text, ReminderForm);
    ReminderTitleLineEdit->setPlaceholderText(tr("全屏提醒标题"));
    ReminderTextLineEdit = new QLineEdit(config.reminder.reminder_small_text, ReminderForm);
    ReminderTextLineEdit->setPlaceholderText(tr("全屏提醒文本"));
    QFormLayout* ReminderFormLayout = new QFormLayout(ReminderForm);
    ReminderFormLayout->setContentsMargins(0, 0, 0, 0);
    ReminderFormLayout->addRow(tr("全屏提醒标题："), ReminderTitleLineEdit);
    ReminderFormLayout->addRow(tr("全屏提醒文本："), ReminderTextLineEdit);
    ReminderTimeListEditor = new ReminderTimeListEditorClass(ReminderSettingsWidget, config);
    ReminderSettingsLayout->addWidget(ReminderForm);
    ReminderSettingsLayout->addWidget(new QLabel(tr("提醒时间："), ReminderSettingsWidget));
    ReminderSettingsLayout->addWidget(ReminderTimeListEditor, 1);
    ReminderSettingsWidget->setEnabled(config.reminder.is_show_reminder);
    ReminderLayout->addWidget(IsShowReminderCheckBox);
    ReminderLayout->addWidget(ReminderSettingsWidget, 1);
    addStep(tr("每天什么时候全屏提醒你？"), tr("到这些时间，会全屏显示离目标时间还剩多少天。"), ReminderBody, { ReminderForm });

    // 事件点
    QWidget* EventPointBody = new QWidget;
    QVBoxLayout* EventPointLayout = new QVBoxLayout(EventPointBody);
    EventPointLayout->setContentsMargins(0, 0, 0, 0);
    EventPointListEditor = new EventPointListEditorClass(EventPointBody, config);
    EventPointLayout->addWidget(EventPointListEditor, 1);
    addStep(tr("要提醒哪些事件点？"), tr("事件点前会出现倒数胶囊，到点时全屏闪烁提醒。点“预览”可以马上看看效果。"), EventPointBody, {});

    for (int i = 0; i < m_steps.size(); i++)
        m_steps[i].number->setText(tr("第 %1 步 / 共 %2 步").arg(i + 1).arg(m_steps.size()));
    m_steps.last().buttons.last()->setText(tr("完成"));

    // 下拉列表用 QListView 才能套用样式表（和设置中心一样）
    for (QComboBox* child : this->findChildren<QComboBox*>()) {
        QListView* ListView = new QListView(child);
        ListView->setAutoFillBackground(false);
        child->setView(ListView);
    }

    // 换页动画：和打开设置中心时一样，色块从左边扫进来盖住，换好页后再从右边扫出去
    SlideWidget = new QWidget(this);
    SlideWidget->hide();
    SlideInAnimation = new QPropertyAnimation(SlideWidget, "pos", this);
    SlideInAnimation->setDuration(250);
    SlideOutAnimation = new QPropertyAnimation(SlideWidget, "pos", this);
    SlideOutAnimation->setDuration(1000);
    SlideOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
    SlideAnimationGroup = new QSequentialAnimationGroup(this);
    SlideAnimationGroup->addAnimation(SlideInAnimation);
    SlideAnimationGroup->addAnimation(SlideOutAnimation);
    connect(SlideInAnimation, &QPropertyAnimation::finished, this, [this] {
        Pages->setCurrentIndex(m_targetIndex);
        });
    connect(SlideAnimationGroup, &QSequentialAnimationGroup::finished, SlideWidget, &QWidget::hide);

    // Connections
    connect(LanguageComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        config.set(config.general.language, LanguageComboBox->itemData(index).toString());
        LanguageLabel->setText(LanguageComboBox->currentText());
        });
    connect(ThemeComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        config.set(config.general.theme, ThemeComboBox->itemData(index).toString());
        ThemeManager::instance().load(config.general.theme); // 发出 changed()，引导和预览都跟着换
        });
    connect(&ThemeManager::instance(), &ThemeManager::changed, this, &OnboardingWizardClass::applyTheme);
    connect(TargetDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [this](const QDateTime& date_time) {
        config.set(config.general.target_date_time, date_time);
        updatePreviewText();
        });
    connect(FloatingBarTextLineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        config.set(config.floating_bar.floating_bar_text, text);
        updatePreviewText();
        });
    connect(IsShowReminderCheckBox, &QCheckBox::checkStateChanged, this, [this] {
        config.set(config.reminder.is_show_reminder, IsShowReminderCheckBox->isChecked());
        ReminderSettingsWidget->setEnabled(config.reminder.is_show_reminder);
        });
    connect(ReminderTitleLineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        config.set(config.reminder.reminder_text, text);
        });
    connect(ReminderTextLineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        config.set(config.reminder.reminder_small_text, text);
        });

    PreviewTimer = new QTimer(this);
    PreviewTimer->setInterval(1000);
    connect(PreviewTimer, &QTimer::timeout, this, &OnboardingWizardClass::updatePreviewText);
    PreviewTimer->start();

    applyTheme();
}

void OnboardingWizardClass::addStep(const QString& title, const QString& subtitle, QWidget* body, const QList<QWidget*>& forms)
{
    Step step;
    step.page = new QWidget(Pages);
    step.layout = new QVBoxLayout(step.page);
    step.number = new QLabel(step.page);
    step.number->setObjectName("OnboardingStepNumber");
    step.title = new QLabel(title, step.page);
    step.title->setObjectName("OnboardingTitle");
    step.title->setWordWrap(true);
    QLabel* SubtitleLabel = new QLabel(subtitle, step.page);
    SubtitleLabel->setObjectName("OnboardingSubtitle");
    SubtitleLabel->setWordWrap(true);
    body->setParent(step.page);
    step.forms = forms;

    QPushButton* BackButton = new QPushButton(tr("上一步"), step.page);
    BackButton->setObjectName("OnboardingBackButton");
    QPushButton* NextButton = new QPushButton(tr("下一步"), step.page);
    NextButton->setObjectName("OnboardingNextButton");
    step.buttons = { BackButton, NextButton };
    QHBoxLayout* ButtonLayout = new QHBoxLayout;
    ButtonLayout->addStretch();
    ButtonLayout->addWidget(BackButton);
    ButtonLayout->addWidget(NextButton);

    step.layout->addWidget(step.number);
    step.layout->addWidget(step.title);
    step.layout->addWidget(SubtitleLabel);
    step.layout->addWidget(body, 1);
    step.layout->addLayout(ButtonLayout);

    const int index = Pages->addWidget(step.page);
    connect(BackButton, &QPushButton::clicked, this, [this, index] { goTo(index - 1); });
    connect(NextButton, &QPushButton::clicked, this, [this, index] {
        if (index + 1 < Pages->count())
            goTo(index + 1);
        else
            finish();
        });
    m_steps.append(step);
}

QWidget* OnboardingWizardClass::createPreview()
{
    // 模拟屏幕顶部的一块，悬浮条贴着它的顶边居中（和悬浮条默认的位置一样）
    QWidget* Desk = new QWidget;
    Desk->setObjectName("OnboardingPreviewDesk");
    Desk->setAttribute(Qt::WA_StyledBackground);
    Desk->setFixedHeight(config.floating_bar.floating_bar_height * 2);
    QVBoxLayout* Layout = new QVBoxLayout(Desk);
    Layout->setContentsMargins(0, 0, 0, 0);
    QLabel* Bar = new QLabel(Desk);
    Bar->setAlignment(Qt::AlignCenter);
    QGraphicsDropShadowEffect* Shadow = new QGraphicsDropShadowEffect(Bar);
    Bar->setGraphicsEffect(Shadow);
    Layout->addWidget(Bar, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_previews.append({ Bar, Shadow });
    return Desk;
}

void OnboardingWizardClass::updatePreviewText()
{
    const QString text = FloatingBarClass::barText(config.floating_bar.floating_bar_text, QDateTime::currentDateTime().secsTo(config.general.target_date_time));
    for (const Preview& preview : m_previews) {
        preview.bar->setText(text);
        preview.bar->setMinimumWidth(0); // QLabel 的 sizeHint 不会小于最小宽度，先放开，否则每次都会多加 20
        preview.bar->setFixedSize(preview.bar->sizeHint().width() + 20, config.floating_bar.floating_bar_height); // 和悬浮条一样，宽度比文字多 20
    }
}

void OnboardingWizardClass::goTo(int index)
{
    if (SlideAnimationGroup->state() == QAbstractAnimation::Running || index < 0 || index >= Pages->count() || m_isFinished)
        return;
    m_targetIndex = index;
    SlideWidget->move(-this->width(), 0);
    SlideWidget->show();
    SlideWidget->raise();
    SlideAnimationGroup->start();
}

void OnboardingWizardClass::finish()
{
    if (SlideAnimationGroup->state() == QAbstractAnimation::Running || m_isFinished)
        return;
    m_isFinished = true;
    config.set(config.general.is_onboarding_finished, true);
    emit finished();
}

void OnboardingWizardClass::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    const int w = this->width();
    const int h = this->height();
    Pages->setGeometry(this->rect());
    SlideWidget->resize(this->size());
    SlideInAnimation->setStartValue(QPoint(-w, 0));
    SlideInAnimation->setEndValue(QPoint(0, 0));
    SlideOutAnimation->setStartValue(QPoint(0, 0));
    SlideOutAnimation->setEndValue(QPoint(w, 0));

    // 字号按高度算（和设置中心一样），标题和语言名称大一些
    QFont font;
    font.setPixelSize(h * 0.025);
    for (QWidget* child : this->findChildren<QWidget*>())
        child->setFont(font);
    QFont numberFont = font;
    numberFont.setBold(true);
    QFont titleFont = font;
    titleFont.setPixelSize(h * 0.05);
    titleFont.setBold(true);
    for (const Step& step : m_steps) {
        step.layout->setContentsMargins(w * 0.06, h * 0.05, w * 0.06, h * 0.04);
        step.layout->setSpacing(h * 0.015);
        step.number->setFont(numberFont);
        step.title->setFont(titleFont);
        for (QWidget* form : step.forms)
            form->setMaximumWidth(w * 0.55);
        for (QPushButton* button : step.buttons)
            button->setMinimumWidth(w * 0.08);
    }
    QFont languageFont = font;
    languageFont.setPixelSize(h * 0.08);
    languageFont.setBold(true);
    LanguageLabel->setFont(languageFont);

    applyTheme();
}

void OnboardingWizardClass::applyTheme()
{
    ThemeManager& theme = ThemeManager::instance();
    // 输入框等控件沿用设置中心的样式，尺寸变量和设置中心一样按高度算
    const qreal unit = this->height() * 0.03;
    const QHash<QString, QString> vars = {
        { "settings_radius", QString("%1px").arg(unit / 4) },
        { "settings_padding", QString("%1px").arg((unit / GOLDEN_RATIO_INV - unit) / 2) },
        { "settings_dropdown_width", QString("%1px").arg(unit / GOLDEN_RATIO_INV) },
        { "settings_spin_button_width", QString("%1px").arg(unit / GOLDEN_RATIO_INV / 2) },
        { "settings_checkbox_size", QString("%1px").arg(qRound(this->height() * 0.02)) },
    };
    this->setStyleSheet(theme.style("SettingsPage") + "\n" + theme.style("SettingsContent", vars) + "\n" + theme.style("OnboardingWizard", vars));
    SlideWidget->setStyleSheet(theme.style("FullscreenSlide"));

    // 预览里的悬浮条：样式、字号和真的悬浮条一样
    QFont barFont;
    barFont.setPixelSize(config.floating_bar.floating_bar_height * GOLDEN_RATIO_INV);
    for (const Preview& preview : m_previews) {
        FloatingBarClass::styleBar(preview.bar, preview.shadow, config.floating_bar.floating_bar_border_radius, config.floating_bar.floating_bar_height, FloatingBarPosition::TopCenter, 0);
        preview.bar->setFont(barFont);
    }
    updatePreviewText();
}
