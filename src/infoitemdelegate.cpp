/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)
* <http://gitorious.org/bangarang>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "infoitemdelegate.h"
#include "mainwindow.h"
#include "sensiblewidgets.h"
#include "platform/mediaitemmodel.h"
#include "platform/infoitemmodel.h"

#include <KGlobalSettings>
#include <KColorScheme>
#include <KIcon>
#include <KIconEffect>
#include <KLineEdit>
#include <KFileDialog>
#include <KDebug>
#include <Soprano/Vocabulary/NAO>
#include <nepomuk/variant.h>
#include <nepomuk/resource.h>
#include <Nepomuk/ResourceManager>

#include <QUrl>
#include <QPalette>
#include <QStyle>
#include <QIcon>
#include <QFont>
#include <QStandardItem>
#include <QApplication>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>

InfoItemDelegate::InfoItemDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
    
}

InfoItemDelegate::~InfoItemDelegate()
{
}

void InfoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = option.rect.height();   
    int padding = 2;
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    
    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());
    
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QStandardItemModel * model = (QStandardItemModel *)index.model();
    bool isEditable = model->itemFromIndex(index)->isEditable();
    if (isEditable && option.state.testFlag(QStyle::State_MouseOver)) {
        KIcon("arrow-left").paint(&p, option.rect.right() - 8, top + (height - 8)/2, 8, 8);
    }
    bool multipleValues = index.data(InfoItemModel::MultipleValuesRole).toBool();
    
    if (index.column() == 0) {
        //Paint first column containing artwork, title, description and field labels
        if (field == "artwork") {
            QIcon artwork = index.data(Qt::DecorationRole).value<QIcon>();
            artwork.paint(&p, option.rect, Qt::AlignCenter, QIcon::Normal);
        } else {
            Qt::AlignmentFlag hAlign;
            QFont textFont = option.font;
            QString text = index.data(Qt::DisplayRole).toString();
            if (field == "title") {
                textFont.setPointSize(1.5*textFont.pointSize());
                hAlign = Qt::AlignHCenter;
                if (multipleValues) {
                    foregroundColor.setAlphaF(0.7);
                    text = i18n("Multiple Values");
                    textFont.setItalic(true);
                } else if (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole)) {
                    textFont.setBold(true);
                }
            } else if (field == "description") {
                textFont = KGlobalSettings::smallestReadableFont();
                hAlign = Qt::AlignJustify;
                padding = 10;
                if (multipleValues) {
                    foregroundColor.setAlphaF(0.7);
                    text = i18n("Multiple Values");
                    textFont.setItalic(true);
                } else if (text.isEmpty()) {
                    foregroundColor.setAlphaF(0.7);
                    text = i18n("No description");
                    textFont.setItalic(true);
                    hAlign = Qt::AlignCenter;
                }
                if (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole)) {
                    textFont.setBold(true);
                }
            } else {
                textFont = KGlobalSettings::smallestReadableFont();
                textFont.setItalic(true);
                hAlign = Qt::AlignRight;
            }
            int textWidth = width - 2 * padding;
            QTextOption textOption(hAlign | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            QRect textRect(left + padding, top, textWidth, height);
            p.setFont(textFont);
            p.setPen(foregroundColor);
            p.drawText(QRectF(textRect), text, textOption);
        }
                                        
    } else if (index.column() == 1) {
        //Paint second column containing field values
        if (multipleValues) {
            int textWidth = width - 2 * padding;
            QFont textFont = KGlobalSettings::smallestReadableFont();
            textFont.setItalic(true);
            p.setFont(textFont);
            foregroundColor.setAlphaF(0.7);
            p.setPen(foregroundColor);
            p.drawText(left + padding, top, textWidth, height,
                       Qt::AlignLeft | Qt::AlignVCenter, i18n("Multiple Values"));
        } else {
            QString text = index.data(Qt::DisplayRole).toString();
            QString field = index.data(InfoItemModel::FieldRole).toString();
            if (field == "audioType") {
                int typeIndex = index.data(Qt::DisplayRole).toInt();
                if (typeIndex == 0) {
                    text = i18n("Music");
                } else if (typeIndex == 1) {
                    text = i18n("Audio Stream");
                } else if (typeIndex == 2) {
                    text = i18n("Audio Clip");
                }
            } else if (field == "videoType") {
                int typeIndex = index.data(Qt::DisplayRole).toInt();
                if (typeIndex == 0) {
                    text = i18n("Movie");
                } else if (typeIndex == 1) {
                    text = i18n("TV Show");
                } else if (typeIndex == 2) {
                    text = i18n("Video Clip");
                }
            }
            QFont textFont = KGlobalSettings::smallestReadableFont();
            if (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole)) {
                textFont.setBold(true);
            }
            int textWidth = width - 2 * padding;
            if (field == "url") {
                text = QFontMetrics(textFont).elidedText(text, Qt::ElideMiddle, textWidth);
            }
            QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            QRect textRect(left + padding, top, textWidth, height);
            p.setFont(textFont);
            p.setPen(foregroundColor);
            p.drawText(QRectF(textRect), text, textOption);
        }
    }
        
    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);
    
}

QSize InfoItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    int padding = 2;
    int width = 0;
    if (index.column() == 0) {
        width = 0.35*m_view->width();
    }
        
    int height;
    if (field == "artwork") {
        height = 128 + 2*padding;
    } else {
        QString text = index.data(Qt::DisplayRole).toString();
        QFont textFont = option.font;
        int availableWidth = 0.65*m_view->width() - 2*padding;
        if (field == "title") { 
            textFont.setPointSize(1.5*textFont.pointSize());
            availableWidth = m_view->width() - 2*padding;
        } else if (field == "description") {
            padding = 10;
            textFont = KGlobalSettings::smallestReadableFont();
            availableWidth = m_view->width() - 2*padding;
        } else if (field == "url") {
            textFont = KGlobalSettings::smallestReadableFont();
            text = QString(); // url text is elided to a single line anyway
        }
        if (availableWidth <= 0) {
            availableWidth = 100;
        }
        height = heightForWordWrap(textFont, availableWidth, text);
    }
       
    return QSize(width, height);
}

bool InfoItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,                                                const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (field == "artwork") {
        if (event->type() == QEvent::MouseButtonDblClick) {
            KUrl newUrl = KFileDialog::getImageOpenUrl(KUrl(), m_parent, i18n("Open artwork file"));
            if (newUrl.isValid()) {
                QPixmap pixmap = QPixmap(newUrl.path()).scaled(QSize(128,128), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                if (!pixmap.isNull()) {
                    model->setData(index, QIcon(pixmap), Qt::DecorationRole);
                    model->setData(index, newUrl, Qt::EditRole);
                    model->setData(index, false, InfoItemModel::MultipleValuesRole);
                }
            }
        }
        return true;
    } else {
        return QItemDelegate::editorEvent(event, model, option, index);
    }
}

QWidget *InfoItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QVariant value = index.data(Qt::EditRole);
    bool multipleValues = index.data(InfoItemModel::MultipleValuesRole).toBool();
    if (value.type() == QVariant::Int) {
        if (field == "audioType") {
            QStringList list;
            list << i18n("Music") << i18n("Audio Stream") << i18n("Audio Clip");
            QComboBox *comboBox = new QComboBox(parent);
            comboBox->setFont(KGlobalSettings::smallestReadableFont());
            comboBox->addItems(list);
            comboBox->setEditable(false);
            comboBox->setCurrentIndex(value.toInt());
            comboBox->setAutoFillBackground(true);
            return comboBox;
        } else if (field == "videoType") {
            QStringList list;
            list << i18n("Movie") << i18n("TV Show") << i18n("Video Clip");
            QComboBox *comboBox = new QComboBox(parent);
            comboBox->setFont(KGlobalSettings::smallestReadableFont());
            comboBox->addItems(list);
            comboBox->setEditable(false);
            comboBox->setCurrentIndex(value.toInt());
            comboBox->setAutoFillBackground(true);
            return comboBox;
        } else {
            QSpinBox *spinBox = new QSpinBox(parent);
            spinBox->setFont(KGlobalSettings::smallestReadableFont());
            if (!multipleValues) spinBox->setValue(value.toInt());
            spinBox->setRange(0,9999);
            spinBox->setAutoFillBackground(true);
            return spinBox;
        }
    } else if (value.type() == QVariant::String) {
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        if (valueList.count() == 0) {
            KLineEdit *lineEdit = new KLineEdit(parent);
            lineEdit->setFont(KGlobalSettings::smallestReadableFont());
            if (!multipleValues) lineEdit->setText(value.toString());
            lineEdit->setAutoFillBackground(true);
            if (field == "tags") {
                lineEdit->setToolTip(i18n("Tags are separated with \";\""));
            }
            return lineEdit;
        } else {
            SComboBox *comboBox = new SComboBox(parent);
            for (int i = 0; i < valueList.count(); i++) {
                comboBox->addItem(valueList.at(i), valueList.at(i));
            }
            comboBox->setEditable(true);
            comboBox->setFont(KGlobalSettings::smallestReadableFont());
            comboBox->setAutoFillBackground(true);
            return comboBox;
        }
    } else if (field != "artwork") {
            KLineEdit *lineEdit = new KLineEdit(parent);
            lineEdit->setFont(KGlobalSettings::smallestReadableFont());
            if (!multipleValues) lineEdit->setText(value.toString());
            lineEdit->setAutoFillBackground(true);
            return lineEdit;
    } else {
        return 0;
    }
    Q_UNUSED(option);
        
}

void InfoItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (field == "audioType" || field == "videoType") {
        QComboBox * comboBox = qobject_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentIndex());
    } else {
        QItemDelegate::setModelData(editor, model, index);
    }
    
    //If the data has changed then make sure the multipleValues flag is set to false
    if (index.data(Qt::EditRole) != index.data(InfoItemModel::OriginalValueRole)) {
        model->setData(index, false, InfoItemModel::MultipleValuesRole); 
    }
}   

void InfoItemDelegate::setView(QAbstractItemView * view) 
{
    m_view = view;
    m_defaultViewSelectionMode = view->selectionMode();
}

int InfoItemDelegate::heightForWordWrap(QFont font, int width, QString text) const
{
    QFontMetrics fm(font);
    int fmWidth = fm.boundingRect(text).width();
    int fmHeight = fm.lineSpacing() + 2;
    int heightMultiplier = 1;
    QString fitText = text;
    while (fmWidth > width) {
        QStringList wordList = fitText.split(QRegExp("\\s+"));
        QString wordWrapText = fitText;
        while (fmWidth > width) {
            wordWrapText.truncate(wordWrapText.lastIndexOf(wordList.takeLast()));
            fmWidth = fm.boundingRect(wordWrapText).width();
        }
        heightMultiplier++;
        if (wordWrapText.isEmpty()) {
            QStringList wordList = fitText.split(QRegExp("\\s+"));
            wordWrapText = wordList.at(0);
        }
        fitText = fitText.mid(wordWrapText.length());
        fmWidth = fm.boundingRect(fitText).width();
    }
    return heightMultiplier*fmHeight;
}
