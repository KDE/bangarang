/* Copyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)
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

#include "infoartistdelegate.h"
#include "mainwindow.h"
#include "sensiblewidgets.h"
#include "platform/mediaitemmodel.h"
#include "platform/infoartistmodel.h"

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
#include <QHeaderView>

InfoArtistDelegate::InfoArtistDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
    
}

InfoArtistDelegate::~InfoArtistDelegate()
{
}

void InfoArtistDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = option.rect.height();   
    int padding = 1;
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    
    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());
    
    bool multipleValues = index.data(Qt::UserRole + 1).toBool();
    if (index.column() == 0) {
        //Paint first column containing artwork, titel and field labels
        if (index.data(InfoArtistModel::FieldRole).toString() == "associatedImage") {
            QIcon artwork = index.data(Qt::DecorationRole).value<QIcon>();
            artwork.paint(&p, option.rect, Qt::AlignCenter, QIcon::Normal);
            //KIcon("download").paint(&p, 0, 0, 22, 22, Qt::AlignCenter, QIcon::Normal);
        } else {
            Qt::AlignmentFlag hAlign = Qt::AlignHCenter;
            QFont textFont = option.font;
            QString text = index.data(Qt::DisplayRole).toString();
            if (index.data(Qt::UserRole).toString() == "fullName") {
                textFont.setPointSize(1.5*textFont.pointSize());
                if (multipleValues) {
                    foregroundColor.setAlphaF(0.7);
                    text = i18n("Multiple Values");
                    textFont.setItalic(true);
                } else if (index.data(Qt::DisplayRole) != index.data(Qt::UserRole + 2)) {
                    textFont.setBold(true);
                }
            } else {
                textFont = KGlobalSettings::smallestReadableFont();
                hAlign = Qt::AlignLeft;
            }
            int textWidth = width - 2 * padding;
            QTextOption textOption(hAlign | Qt::AlignVCenter);
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

QSize InfoArtistDelegate::sizeHint(const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    QString field = index.data(InfoArtistModel::FieldRole).toString();
    int padding = 2;
    int width = 0;
    int height;
    if (field == "associatedImage") {
        QIcon artwork = index.data(Qt::DecorationRole).value<QIcon>();
        height = artwork.actualSize(QSize(200,200)).height() + 2*padding;
        //height = 200 + 2*padding;
    } else {
        QFont textFont = option.font;
        int availableWidth = m_view->width() - 2*padding;
        if (field == "fullName") { 
            textFont.setPointSize(1.5*textFont.pointSize());
        } else{
            textFont = KGlobalSettings::smallestReadableFont();
        }
        QFontMetrics fm(textFont);
        int fmWidth = fm.boundingRect(index.data(Qt::DisplayRole).toString()).width();
        int heightMultiplier = 1 + (fmWidth / availableWidth);
        height = heightMultiplier * fm.height() + 2 * padding;
    }
       
    return QSize(width, height);
}

int InfoArtistDelegate::columnWidth (int column) const {
    if (column == 0) {
        return 70;
    } else {
        return m_view->width() - columnWidth(0);
    }
}

bool InfoArtistDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,                                                const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QString field = index.data(InfoArtistModel::FieldRole).toString();
    if (field == "associatedImage") {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent * mouseEvent = (QMouseEvent *)event;
            int downloadRight = option.rect.left() +22;
            int downloadBottom = option.rect.top() + 22;
            if ((mouseEvent->x() <= downloadRight) && (mouseEvent->y() <= downloadBottom)) {
                InfoArtistModel * artistModel = (InfoArtistModel *)model;
                artistModel->downloadInfo();
                m_view->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
                m_view->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
                
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            KUrl newUrl = KFileDialog::getImageOpenUrl(KUrl(), m_parent, i18n("Open artwork file"));
            if (newUrl.isValid()) {
                QPixmap pixmap = QPixmap(newUrl.path()).scaled(QSize(200,200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                if (!pixmap.isNull()) {
                    model->setData(index, QIcon(pixmap), Qt::DecorationRole);
                    model->setData(index, newUrl, Qt::EditRole);
                }
            }
        }
        return true;
    } else {
        return QItemDelegate::editorEvent(event, model, option, index);
    }
}

QWidget *InfoArtistDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QString field = index.data(InfoArtistModel::FieldRole).toString();
    QVariant value = index.data(Qt::EditRole);
    bool multipleValues = index.data(InfoArtistModel::MultipleValuesRole).toBool();
    if (value.type() == QVariant::String) {
        if (field == "description") {
            QTextEdit *textEdit = new QTextEdit(parent);
            textEdit->setFont(KGlobalSettings::smallestReadableFont());
            if (!multipleValues) textEdit->setText(value.toString());
            textEdit->setAutoFillBackground(true);
            return textEdit;
        } else {
            QStringList valueList = index.data(InfoArtistModel::ValueListRole).toStringList();
            if (valueList.count() == 0) {
                KLineEdit *lineEdit = new KLineEdit(parent);
                lineEdit->setFont(KGlobalSettings::smallestReadableFont());
                if (!multipleValues) lineEdit->setText(value.toString());
                lineEdit->setAutoFillBackground(true);
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
        }
    } else if (field != "associatedImage") {
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

void InfoArtistDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const
{
    QItemDelegate::setModelData(editor, model, index);
    
    //If the data has changed then make sure the multipleValues flag is set to false
    if (index.data(Qt::EditRole) != index.data(Qt::UserRole+2)) {
        model->setData(index, false, Qt::UserRole+1); 
    }
}   

void InfoArtistDelegate::setView(QTableView * view) 
{
    m_view = view;
    m_defaultViewSelectionMode = view->selectionMode();
}