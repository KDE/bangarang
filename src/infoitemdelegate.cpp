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
#include "ui_mainwindow.h"
#include "sensiblewidgets.h"
#include "platform/mediaitemmodel.h"
#include "platform/infoitemmodel.h"
#include "platform/utilities.h"

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
#include <math.h>

InfoItemDelegate::InfoItemDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
    
    m_nepomukInited = Utilities::nepomukInited();

    for (int i = 0; i < 19; i++) {
        if (i%2) {
            m_artworkRotations.append(10);
        } else {
            m_artworkRotations.append(-10);
        }
    }
}

InfoItemDelegate::~InfoItemDelegate()
{
}

void InfoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStandardItemModel * model = (QStandardItemModel *)index.model();

    //Get basic information about painting area
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = option.rect.height();   
    QColor foregroundColor = option.palette.color(QPalette::Text);
    QColor hoverColor = option.palette.color(QPalette::Highlight);
    hoverColor.setAlpha(35);

    //Get basic information about field
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QString text = index.data(Qt::DisplayRole).toString();
    bool multipleValues = index.data(InfoItemModel::MultipleValuesRole).toBool();
    bool isEditable = model->itemFromIndex(index)->isEditable();
    bool modified = (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole));
    bool isArtwork = (field == "artwork");

    //Set basic formatting info
    int padding = 3;
    QFont textFont = KGlobalSettings::smallestReadableFont();
    Qt::AlignmentFlag hAlign = Qt::AlignLeft;
    int fieldNameWidth = qMax(70, (width - 4 * padding)/4);

    //Formatting modifications based on field info
    if (multipleValues) {
        text = i18n("Multiple Values");
        textFont.setItalic(true);
        foregroundColor.setAlphaF(0.7);
    }
    if (modified) {
        textFont.setBold(true);
    }

    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(-option.rect.topLeft());
    
    if (index.column() == 0) {
        //Special handling for some field types
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
        } else if (field == "artwork") {
            fieldNameWidth = 0;
        } else if (field == "title") {
            textFont = option.font;
            textFont.setPointSize(1.5*textFont.pointSize());
            fieldNameWidth = 0;
            hAlign = Qt::AlignHCenter;
        } else if (field == "description") {
            hAlign = Qt::AlignJustify;
            padding = 10;
            fieldNameWidth = 0;
            if (text.isEmpty()) {
                foregroundColor.setAlphaF(0.7);
                text = i18n("No description");
                hAlign = Qt::AlignCenter;
                textFont.setItalic(true);
            }
        } else if (field == "url") {
            int textWidth = width - 5 * padding - fieldNameWidth;
            text = QFontMetrics(textFont).elidedText(text, Qt::ElideMiddle, textWidth);
        }

        //Paint field name
        if (fieldNameWidth > 0) {
            QString fieldName = index.data(InfoItemModel::FieldNameRole).toString();
            QFont fieldNameFont = KGlobalSettings::smallestReadableFont();
            fieldNameFont.setItalic(true);
            QTextOption textOption(Qt::AlignRight | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            QRect textRect(left + padding, top, fieldNameWidth, height);
            p.setFont(fieldNameFont);
            p.setPen(foregroundColor);
            p.drawText(QRectF(textRect), fieldName, textOption);
        }

        //Paint field data
        if (isArtwork) {
            if (isEditable && option.state.testFlag(QStyle::State_MouseOver)) {
                p.save();
                QRect hoverRect(left+padding, top, width-2*padding, height);
                p.setPen(Qt::NoPen);
                p.setBrush(QBrush(hoverColor));
                p.drawRoundedRect(hoverRect, 3.0, 3.0);
                p.restore();
            }
            QList<QVariant> artworkList = index.data(InfoItemModel::ArtworkListRole).toList();
            if (artworkList.count() == 0) {
                QIcon artwork = index.data(Qt::DecorationRole).value<QIcon>();
                artwork.paint(&p, left+padding, top+padding, width -2*padding, height-2*padding, Qt::AlignCenter, QIcon::Normal);
            } else {
                p.save();
                int artworkSize  = 100;
                if (artworkList.count() == 1) {
                    artworkSize = 128;
                }
                int spacing = (width - padding - artworkSize - 30)/artworkList.count();
                int aTop = top + (height-artworkSize)/2;
                int startx = (width/2) - ((artworkSize/2) - (spacing/2)*(artworkList.count()-1));
                p.translate(startx, aTop);
                for (int i = artworkList.count()-1; i >= 0; i--) {
                    qreal rot = m_artworkRotations.at(i);
                    if (artworkList.count() == 1) {
                        rot = 0;
                    }
                    double rotRad = (3.14159/180)*rot;
                    qreal r = (sqrt(2.0*artworkSize*artworkSize))/2.0;
                    int transX = (artworkSize/2) - r*cos(rotRad +(3.14159/4));
                    int transY = r*sin(rotRad + (3.14159/4)) - (artworkSize/2);
                    p.rotate(rot);
                    p.translate(transX, -transY);
                    p.fillRect(0,0,artworkSize, artworkSize, Qt::white);
                    p.drawPixmap(0, 0, artworkSize, artworkSize, artworkList.at(i).value<QPixmap>());
                    if (artworkList.count() > 1) {
                        QColor outlineColor = QColor(Qt::black);
                        outlineColor.setAlphaF(0.7);
                        p.setPen(outlineColor);
                        p.drawRect(0, 0, artworkSize, artworkSize);
                    }
                    p.translate(-transX, transY);
                    p.rotate(-rot);
                    p.translate(-spacing, 0);
                }
                p.restore();
            }
        } else {
            int textWidth = width - 5 * padding - fieldNameWidth;
            int textLeft = left + fieldNameWidth + 3 * padding;
            if (fieldNameWidth == 0) {
                textWidth = width - 4 * padding;
                textLeft = left + 2*padding;
            }
            QTextOption textOption(hAlign | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            QRect textRect(textLeft, top, textWidth, height);
            if (isEditable && option.state.testFlag(QStyle::State_MouseOver)) {
                p.save();
                QRect hoverRect(textLeft-padding, top, textWidth+2*padding, height);
                p.setPen(Qt::NoPen);
                p.setBrush(QBrush(hoverColor));
                p.drawRoundedRect(hoverRect, 3.0, 3.0);
                p.restore();
            }
            p.setFont(textFont);
            p.setPen(foregroundColor);
            p.drawText(QRectF(textRect), text, textOption);
        }

        //Paint little arrow indicating field is editable
        if (isEditable && option.state.testFlag(QStyle::State_MouseOver)) {
            if (isArtwork) {
                //Show clear field "button" when artworkUrl is specified
                QString artworkUrl = index.data(Qt::EditRole).toString();
                if (!artworkUrl.isEmpty()) {
                    int clrTop = top + (height - 16)/2;
                    int clrLeft = option.rect.right() - 16;
                    KIcon("edit-clear-locationbar-rtl").paint(&p, clrLeft, clrTop, 16, 16);
                }
            }
        }
    }
    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);    
}

QSize InfoItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    Q_UNUSED(option);
    return QSize(0, rowHeight(index.row()));
}

bool InfoItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QPoint mousePos = ((QMouseEvent *)event)->pos();
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (field == "artwork") {
        if (event->type() == QEvent::MouseButtonRelease) {
            KUrl newUrl = KFileDialog::getImageOpenUrl(KUrl(), m_parent, i18n("Open artwork file"));
            if (newUrl.isValid()) {
                model->setData(index, newUrl.url(), Qt::EditRole);
                model->setData(index, false, InfoItemModel::MultipleValuesRole);
            }
            m_parent->ui->mediaView->setFocus();
        } else if (event->type() == QEvent::MouseButtonPress) {
            QRect clearButtonRect = QRect(option.rect.left()+option.rect.width()-16, option.rect.top()+(option.rect.height()-16)/2, 16, 16);
            if (clearButtonRect.contains(mousePos)) {
                model->setData(index, QString(""), Qt::EditRole);
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
    if (field == "artwork") {
        return 0;
    }
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

void InfoItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    int padding = 3;
    int fieldNameWidth = qMax(70, (option.rect.width() - 4 * padding)/4);
    if (field == "artwork" || field == "title" || field == "description") {
        fieldNameWidth = -padding;
    }
    QRect editorRect = QRect(option.rect.left() + fieldNameWidth + 2 * padding,
                             option.rect.top(),
                             option.rect.width() - (fieldNameWidth + 2 * padding),
                             option.rect.height());
    editor->setGeometry(editorRect);
}

void InfoItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (field == "audioType" || field == "videoType") {
        QComboBox * comboBox = qobject_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentIndex(), Qt::EditRole);
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
    int fmHeight = fm.lineSpacing() + 1;
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

int InfoItemDelegate::rowHeight(int row) const
{
    QModelIndex index = m_view->model()->index(row,0);
    QString field = index.data(InfoItemModel::FieldRole).toString();
    bool modified = (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole));
    int padding = 3;
    int width = m_view->width();

    int height;
    if (field == "artwork") {
        height = 128 + 10 + 2*padding ; //10 pixel to accomodate rotated artwork
    } else {
        QString text = index.data(Qt::DisplayRole).toString();
        QFont textFont = KGlobalSettings::smallestReadableFont();
        if (modified) {
            textFont.setBold(true);
        }
        int fieldNameWidth = qMax(70, (width - 4 * padding)/4);
        int availableWidth = width - fieldNameWidth - 2*padding;
        if (field == "title") {
            textFont = QFont();
            textFont.setPointSize(1.5*textFont.pointSize());
            availableWidth = width - 2*padding;
        } else if (field == "description") {
            availableWidth = width - 2*10;
        } else if (field == "url") {
            textFont = KGlobalSettings::smallestReadableFont();
            text = QString(); // url text is elided to a single line anyway
        }
        if (availableWidth <= 0) {
            availableWidth = 100;
        }
        if (modified) {
            textFont.setBold(true);
        }
        height = heightForWordWrap(textFont, availableWidth, text) + 2*padding;
    }

    return height;
}

int InfoItemDelegate::heightForAllRows()
{
    int height = 0;
    for (int i = 0; i < m_view->model()->rowCount(); i++) {
        height = height + rowHeight(i);
    }
    return height + 2;
}

