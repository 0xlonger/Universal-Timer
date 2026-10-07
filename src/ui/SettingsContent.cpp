#include "core/Global.h"
#include "ui/SettingsContent.h"
#include "ui/HideRulesEditor.h"
#include "core/ThemeManager.h"

#include <QListView>
#include <QFormLayout>
#include <QMessageBox>

SettingsContentClass::SettingsContentClass(QWidget* parent, ConfigManager& cfg, FloatingBarClass* bar)
    : QStackedWidget(parent), config(cfg), FloatingBar(bar)
{

    initializeObjects();
    connectEmissions();

}

SettingsContentClass::~SettingsContentClass()
{}

void SettingsContentClass::setCurrentPage(const Page& page) {
    switch (page) {
        case Page::None: this->setCurrentIndex(0); break;
        case Page::GeneralSettingsPage: this->setCurrentWidget(GeneralSettingsPage); break;
        case Page::FloatingBarSettingsPage: this->setCurrentWidget(FloatingBarSettingsPage); break;
        case Page::ReminderSettingsPage: this->setCurrentWidget(ReminderSettingsPage); break;
        case Page::DonatePage: this->setCurrentWidget(DonatePage); break;
        case Page::AboutPage: this->setCurrentWidget(AboutPage); break;
    }
}

void SettingsContentClass::initializeObjects() {
    // Pages
    GeneralSettingsPage = new QWidget(this);
    FloatingBarSettingsPage = new QWidget(this);
    ReminderSettingsPage = new QWidget(this);
    DonatePage = new DonatePageClass(this);
    AboutPage = new AboutPageClass(this);

    // Settings Items
    // General
    TargetDateTimeEdit = new QDateTimeEdit(config.general.target_date_time, GeneralSettingsPage);
    TargetDateTimeEdit->setCalendarPopup(true);
    TargetDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss"); // 设置显示格式
    LanguageComboBox = new QComboBox(GeneralSettingsPage);
    ThemeComboBox = new QComboBox(GeneralSettingsPage);
    for (const ThemeManager::ThemeInfo& theme : ThemeManager::instance().themes())
        ThemeComboBox->addItem(theme.name, theme.id);
    ThemeComboBox->setCurrentIndex(qMax(0, ThemeComboBox->findData(ThemeManager::instance().currentId())));

    // Floating Bar
    IsShowFloatingBarCheckBox = new QCheckBox(tr("是否显示悬浮条"), FloatingBarSettingsPage);
    IsShowFloatingBarCheckBox->setChecked(config.floating_bar.is_show_floating_bar);
    FloatingBarTextLineEdit = new QLineEdit(config.floating_bar.floating_bar_text, FloatingBarSettingsPage);
    FloatingBarTextLineEdit->setPlaceholderText(tr("悬浮条文本"));
    FloatingBarLevelComboBox = new QComboBox(FloatingBarSettingsPage);
    FloatingBarLevelComboBox->addItems({tr("置顶"), tr("置底")});
    FloatingBarLevelComboBox->setCurrentIndex(config.floating_bar.floating_bar_on_top ? 0 : 1);
    FloatingBarPositionComboBox = new QComboBox(FloatingBarSettingsPage);
    FloatingBarPositionComboBox->addItems({tr("左上"), tr("中上"), tr("右上")});
    FloatingBarPositionComboBox->setCurrentIndex(config.floating_bar.floating_bar_position == FloatingBarPosition::TopLeft ? 0 : (config.floating_bar.floating_bar_position == FloatingBarPosition::TopCenter ? 1 : 2 ));
    FloatingBarHeightSpinBox = new QSpinBox(FloatingBarSettingsPage);
    FloatingBarHeightSpinBox->setRange(0, 5714);
    FloatingBarHeightSpinBox->setValue(config.floating_bar.floating_bar_height);
    FloatingBarHeightSpinBox->setSuffix(tr(" 像素")); // 悬浮条高度后缀文本
    FloatingBarBorderRadiusSpinBox = new QSpinBox(FloatingBarSettingsPage);
    FloatingBarBorderRadiusSpinBox->setRange(0, config.floating_bar.floating_bar_height / 2);
    FloatingBarBorderRadiusSpinBox->setValue(config.floating_bar.floating_bar_border_radius);
    FloatingBarBorderRadiusSpinBox->setSuffix(tr(" 像素")); // 悬浮条圆角半径后缀文本
    FloatingBarTopMarginSpinBox = new QSpinBox(FloatingBarSettingsPage);
    FloatingBarTopMarginSpinBox->setRange(0, 5714);
    FloatingBarTopMarginSpinBox->setValue(config.floating_bar.floating_bar_top_margin);
    FloatingBarTopMarginSpinBox->setSuffix(tr(" 像素")); // 悬浮条距顶边距离后缀文本
    FloatingBarOpacitySpinBox = new QSpinBox(FloatingBarSettingsPage);
    FloatingBarOpacitySpinBox->setRange(0, 100);
    FloatingBarOpacitySpinBox->setValue(config.floating_bar.floating_bar_opacity);
    FloatingBarOpacitySpinBox->setSuffix(tr(" %")); // 悬浮条不透明度后缀文本
    IsMouseInFadingCheckBox = new QCheckBox(tr("鼠标移入时淡化"), FloatingBarSettingsPage);
    IsMouseInFadingCheckBox->setChecked(config.floating_bar.is_mouse_in_fading_enabled);
    IsMouseClickThroughCheckBox = new QCheckBox(tr("鼠标点击穿透"), FloatingBarSettingsPage);
    IsMouseClickThroughCheckBox->setChecked(config.floating_bar.is_mouse_click_through_enabled);
    FloatingBarHideModeComboBox = new QComboBox(FloatingBarSettingsPage);
    FloatingBarHideModeComboBox->addItems({tr("基础模式"), tr("高级模式（规则集）")});
    FloatingBarHideModeComboBox->setCurrentIndex(config.floating_bar.floating_bar_hide_mode == FloatingBarHideMode::Advanced ? 1 : 0);
    HideOnMaxWindowCheckBox = new QCheckBox(tr("前台是最大化窗口时隐藏"), FloatingBarSettingsPage);
    HideOnMaxWindowCheckBox->setChecked(config.floating_bar.hide_on_max_window);
    HideOnFullscreenCheckBox = new QCheckBox(tr("前台是全屏窗口时隐藏"), FloatingBarSettingsPage);
    HideOnFullscreenCheckBox->setChecked(config.floating_bar.hide_on_fullscreen);
    EditHideRulesButton = new QPushButton(tr("编辑规则集…"), FloatingBarSettingsPage);

    // Reminder
    IsShowReminderCheckBox = new QCheckBox(tr("是否显示全屏提醒"), ReminderSettingsPage);
    IsShowReminderCheckBox->setChecked(config.reminder.is_show_reminder);
    ReminderTitleLineEdit = new QLineEdit(config.reminder.reminder_text, ReminderSettingsPage);
    ReminderTitleLineEdit->setPlaceholderText(tr("全屏提醒标题"));
    ReminderTextLineEdit = new QLineEdit(config.reminder.reminder_small_text, ReminderSettingsPage);
    ReminderTextLineEdit->setPlaceholderText(tr("全屏提醒文本"));
    ReminderRemainingDaysToPlayCountdownSoundSpinBox = new QSpinBox(ReminderSettingsPage);
    ReminderRemainingDaysToPlayCountdownSoundSpinBox->setRange(INT_MIN, INT_MAX);
    ReminderRemainingDaysToPlayCountdownSoundSpinBox->setValue(config.reminder.remaining_days_to_play_countdown_sound);
    ReminderRemainingDaysToPlayCountdownSoundSpinBox->setPrefix(tr("剩余天数≤ ")); // 剩余天数播放倒计时音效前缀文本
    ReminderRemainingDaysToPlayCountdownSoundSpinBox->setSuffix(tr(" 天时播放倒计时提醒音")); // 剩余天数播放倒计时音效后缀文本
    ReminderRemainingDaysToPlayHeartbeatSoundSpinBox = new QSpinBox(ReminderSettingsPage);
    ReminderRemainingDaysToPlayHeartbeatSoundSpinBox->setRange(INT_MIN, INT_MAX);
    ReminderRemainingDaysToPlayHeartbeatSoundSpinBox->setValue(config.reminder.remaining_days_to_play_heartbeat_sound);
    ReminderRemainingDaysToPlayHeartbeatSoundSpinBox->setPrefix(tr("剩余天数≤ ")); // 剩余天数播放心跳音效前缀文本
    ReminderRemainingDaysToPlayHeartbeatSoundSpinBox->setSuffix(tr(" 天时播放心跳提醒音")); // 剩余天数播放心跳音效后缀文本
    ReminderBlockShowTimesSpinBox = new QSpinBox(ReminderSettingsPage);
    ReminderBlockShowTimesSpinBox->setRange(0, USHRT_MAX);
    ReminderBlockShowTimesSpinBox->setValue(config.reminder.block_show_times);
    ReminderBlockShowTimesSpinBox->setSuffix(tr(" 次")); // 提醒音播放次数和方块闪烁次数后缀文本
    ReminderPreviewButton= new QPushButton(tr("预览"), ReminderSettingsPage);

    for (QComboBox* child : this->findChildren<QComboBox*>()) {
        QListView* ListView = new QListView(child);
        ListView->setAutoFillBackground(false);
        child->setView(ListView);
    }

    for (QWidget* child : FloatingBarSettingsPage->findChildren<QWidget*>())
        if (child != IsShowFloatingBarCheckBox)
            child->setEnabled(config.floating_bar.is_show_floating_bar);
    updateHideModeWidgets();

    for (QWidget* child : ReminderSettingsPage->findChildren<QWidget*>())
        if (child != IsShowReminderCheckBox)
            child->setEnabled(config.reminder.is_show_reminder);

    QFormLayout* GeneralSettingsPageLayout = new QFormLayout(GeneralSettingsPage);
    GeneralSettingsPageLayout->addRow(tr("目标时间："), TargetDateTimeEdit);
    GeneralSettingsPageLayout->addRow(tr("语言："), LanguageComboBox);
    GeneralSettingsPageLayout->addRow(tr("主题："), ThemeComboBox);
    GeneralSettingsPageLayout->setContentsMargins(25, 25, 25, 25);
    GeneralSettingsPage->setLayout(GeneralSettingsPageLayout);

    QFormLayout* FloatingBarSettingsPageLayout = new QFormLayout(FloatingBarSettingsPage);
    FloatingBarSettingsPageLayout->addRow(IsShowFloatingBarCheckBox);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条文本："), FloatingBarTextLineEdit);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条层级："), FloatingBarLevelComboBox);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条位置："), FloatingBarPositionComboBox);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条高度："), FloatingBarHeightSpinBox);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条圆角半径："), FloatingBarBorderRadiusSpinBox);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条距顶边距离："), FloatingBarTopMarginSpinBox);
    FloatingBarSettingsPageLayout->addRow(tr("悬浮条不透明度："), FloatingBarOpacitySpinBox);
    FloatingBarSettingsPageLayout->addRow(IsMouseInFadingCheckBox);
    FloatingBarSettingsPageLayout->addRow(IsMouseClickThroughCheckBox);
    FloatingBarSettingsPageLayout->addRow(tr("隐藏悬浮条："), FloatingBarHideModeComboBox);
    FloatingBarSettingsPageLayout->addRow(HideOnMaxWindowCheckBox);
    FloatingBarSettingsPageLayout->addRow(HideOnFullscreenCheckBox);
    FloatingBarSettingsPageLayout->addRow(EditHideRulesButton);
    FloatingBarSettingsPageLayout->setContentsMargins(25, 25, 25, 25);
    FloatingBarSettingsPage->setLayout(FloatingBarSettingsPageLayout);

    QFormLayout* ReminderSettingsPageLayout = new QFormLayout(ReminderSettingsPage);
    ReminderSettingsPageLayout->addRow(IsShowReminderCheckBox);
    ReminderSettingsPageLayout->addRow(tr("全屏提醒标题："), ReminderTitleLineEdit);
    ReminderSettingsPageLayout->addRow(tr("全屏提醒文本："), ReminderTextLineEdit);
    ReminderSettingsPageLayout->addRow(ReminderRemainingDaysToPlayCountdownSoundSpinBox);
    ReminderSettingsPageLayout->addRow(ReminderRemainingDaysToPlayHeartbeatSoundSpinBox);
    ReminderSettingsPageLayout->addRow(tr("提醒音播放次数和方块闪烁次数："), ReminderBlockShowTimesSpinBox);
    ReminderSettingsPageLayout->addRow(ReminderPreviewButton);
    ReminderSettingsPageLayout->setContentsMargins(25, 25, 25, 25);
    ReminderSettingsPage->setLayout(ReminderSettingsPageLayout);

    this->addWidget(new QWidget(this));
    this->addWidget(GeneralSettingsPage);
    this->addWidget(FloatingBarSettingsPage);
    this->addWidget(ReminderSettingsPage);
    this->addWidget(DonatePage);
    this->addWidget(AboutPage);
    this->setCurrentPage(Page::None);
}

void SettingsContentClass::connectEmissions() {

    // General
    connect(TargetDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [this](const QDateTime& date_time) {
        config.set(config.general.target_date_time, date_time);
        });
    // todo)) update_interval...
    connect(ThemeComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        config.set(config.general.theme, ThemeComboBox->itemData(index).toString());
        ThemeManager::instance().load(config.general.theme);
        });
    connect(&ThemeManager::instance(), &ThemeManager::changed, this, &SettingsContentClass::applyTheme);

    // FloatingBar
    connect(IsShowFloatingBarCheckBox, &QCheckBox::checkStateChanged, this, [this] {
        config.set(config.floating_bar.is_show_floating_bar, IsShowFloatingBarCheckBox->isChecked());
        FloatingBar->updateVisibility();
        for (QWidget* child : FloatingBarSettingsPage->findChildren<QWidget*>())
            if (child != IsShowFloatingBarCheckBox)
                child->setEnabled(config.floating_bar.is_show_floating_bar);
        updateHideModeWidgets();
        });
    connect(FloatingBarTextLineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        config.set(config.floating_bar.floating_bar_text, text);
        });
    connect(FloatingBarLevelComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        config.set(config.floating_bar.floating_bar_on_top, index == 0);
        FloatingBar->updateWindowFlags();
        FloatingBar->hide();
        FloatingBar->updateVisibility();
#ifdef Q_OS_WIN
        if (config.floating_bar.floating_bar_on_top) {
            QMessageBox::information(this, tr("提示"), tr("已设置悬浮条置顶，需要重新打开程序才可生效。"));
        }
#endif
        });
    connect(FloatingBarPositionComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        config.set(config.floating_bar.floating_bar_position, index == 0 ? FloatingBarPosition::TopLeft : (index == 1 ? FloatingBarPosition::TopCenter : FloatingBarPosition::TopRight));
        });
    connect(FloatingBarHeightSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.floating_bar.floating_bar_height, value);
        FloatingBar->setFixedHeight(value);
        FloatingBar->applyTheme(config.floating_bar.floating_bar_border_radius, value);
        QFont font;
        font.setPixelSize(value * GOLDEN_RATIO_INV);
        FloatingBar->Bar->setFont(font);
        FloatingBarBorderRadiusSpinBox->setMaximum(value / 2);
        });
    connect(FloatingBarBorderRadiusSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.floating_bar.floating_bar_border_radius, value);
        FloatingBar->applyTheme(value, config.floating_bar.floating_bar_height); // 更新悬浮条样式
        });
    connect(FloatingBarTopMarginSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.floating_bar.floating_bar_top_margin, value);
        });
    connect(FloatingBarOpacitySpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.floating_bar.floating_bar_opacity, value);
        FloatingBar->updateOpacity();
        });
    connect(IsMouseInFadingCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        config.set(config.floating_bar.is_mouse_in_fading_enabled, checked);
        FloatingBar->updateOpacity();
        });
    connect(IsMouseClickThroughCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        config.set(config.floating_bar.is_mouse_click_through_enabled, checked);
        FloatingBar->updateWindowFlags();
        });
    connect(FloatingBarHideModeComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        config.set(config.floating_bar.floating_bar_hide_mode, index == 1 ? FloatingBarHideMode::Advanced : FloatingBarHideMode::Basic);
        updateHideModeWidgets();
        });
    connect(HideOnMaxWindowCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        config.set(config.floating_bar.hide_on_max_window, checked);
        });
    connect(HideOnFullscreenCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        config.set(config.floating_bar.hide_on_fullscreen, checked);
        });
    connect(EditHideRulesButton, &QPushButton::clicked, this, [this] {
        HideRulesEditorClass* editor = new HideRulesEditorClass(this, config, FloatingBar, EditHideRulesButton->font());
        editor->setAttribute(Qt::WA_DeleteOnClose);
        editor->open();
        });

    // Reminder
    connect(IsShowReminderCheckBox, &QCheckBox::checkStateChanged, this, [this] {
        config.set(config.reminder.is_show_reminder, IsShowReminderCheckBox->isChecked());
        for (QWidget* child : ReminderSettingsPage->findChildren<QWidget*>())
            if (child != IsShowReminderCheckBox)
                child->setEnabled(config.reminder.is_show_reminder);
        });
    connect(ReminderTitleLineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        config.set(config.reminder.reminder_text, text);
        });
    connect(ReminderTextLineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        config.set(config.reminder.reminder_small_text, text);
        });
    connect(ReminderRemainingDaysToPlayCountdownSoundSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.reminder.remaining_days_to_play_countdown_sound, value);
        });
    connect(ReminderRemainingDaysToPlayHeartbeatSoundSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.reminder.remaining_days_to_play_heartbeat_sound, value);
        });
    connect(ReminderBlockShowTimesSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
        config.set(config.reminder.block_show_times, value);
        });
    connect(ReminderPreviewButton, &QPushButton::clicked, this, [this] {
        emit clickedReminderPreviewButton();
        });

}

void SettingsContentClass::updateHideModeWidgets() {
    const bool enabled = config.floating_bar.is_show_floating_bar;
    const bool advanced = config.floating_bar.floating_bar_hide_mode == FloatingBarHideMode::Advanced;
    HideOnMaxWindowCheckBox->setEnabled(enabled && !advanced);
    HideOnFullscreenCheckBox->setEnabled(enabled && !advanced);
    EditHideRulesButton->setEnabled(enabled && advanced);
}

void SettingsContentClass::resizeEvent(QResizeEvent* event) {
    QStackedWidget::resizeEvent(event);
    
    QFont font;
    font.setPixelSize(this->height() * 0.025);
    for (QWidget* child : this->findChildren<QWidget*>()) {
        child->setFont(font);
    }

    applyTheme();
}

void SettingsContentClass::applyTheme() {
    // 尺寸按设置中心高度算，作为变量提供给主题
    const qreal unit = this->height() * 0.03;
    this->setStyleSheet(ThemeManager::instance().style("SettingsContent", {
        { "settings_radius", QString("%1px").arg(unit / 4) },
        { "settings_padding", QString("%1px").arg((unit / GOLDEN_RATIO_INV - unit) / 2) },
        { "settings_dropdown_width", QString("%1px").arg(unit / GOLDEN_RATIO_INV) },
        { "settings_spin_button_width", QString("%1px").arg(unit / GOLDEN_RATIO_INV / 2) },
        }));
}