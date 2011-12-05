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
#include "infoitemview.h"
#include "../common/mainwindow.h"
#include "ui_mainwindow.h"
#include "../common/sensiblewidgets.h"
#include "../common/bangarangapplication.h"
#include "medialistsmanager.h"
#include "../common/starrating.h"
#include "../common/artworkpainter.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/infoitemmodel.h"
#include "../../platform/utilities/utilities.h"
#include "../../platform/playlist.h"

#include <KGlobalSettings>
#include <KColorScheme>
#include <KIcon>
#include <KIconEffect>
#include <KLineEdit>
#include <KFileDialog>
#include <KDateTime>
#include <KDebug>
#include <Soprano/Vocabulary/NAO>
#include <nepomuk/variant.h>
#include <nepomuk/resource.h>
#include <Nepomuk/ResourceManager>

#include <QUrl>
#include <QDesktopServices>
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
    m_application = (BangarangApplication *)KApplication::kApplication();

    m_nepomukInited = Utilities::nepomukInited();

    m_stringListIndexEditing = -1;

    m_rowOfNewValue = -1;

    m_padding = 3;

    m_isEditing = false;

    m_typeChanged = false;

    m_starRatingSize = StarRating::Big;

    m_suppressEditing = false;

    m_drillIcon = KIcon("bangarang-category-browse");
    QImage drillIconHighlightImage = KIcon("bangarang-category-browse").pixmap(16+m_padding,16+m_padding).toImage();
    KIconEffect::toGamma(drillIconHighlightImage, 0.5);
    m_drillIconHighlight = QIcon(QPixmap::fromImage(drillIconHighlightImage));

    connect(this, SIGNAL(closeEditor(QWidget*)), this, SLOT(endEditing(QWidget*)));
}

InfoItemDelegate::~InfoItemDelegate()
{
}

void InfoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStandardItemModel * model = (QStandardItemModel *)index.model();

    //Get basic information about painting area
    const int width = option.rect.width();
    const int height = option.rect.height();
    QColor foregroundColor = option.palette.color(QPalette::Text);
    QColor hoverColor = option.palette.color(QPalette::Highlight);
    hoverColor.setAlpha(35);

    //Get basic information about field
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QString text;
    if (!index.data(Qt::DisplayRole).isNull()) {
        if (index.data(Qt::DisplayRole).type() == QVariant::DateTime) {
            KDateTime dateTime(index.data(Qt::DisplayRole).toDateTime());
            text = KGlobal::locale()->formatDateTime(dateTime.toLocalZone(), KLocale::FancyLongDate);
        } else {
            text  = index.data(Qt::DisplayRole).toString();
        }
    }
    bool multipleValues = index.data(InfoItemModel::MultipleValuesRole).toBool();
    bool isEditable = model->itemFromIndex(index)->isEditable() && !m_suppressEditing;
    bool modified = (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole));
    bool isArtwork = (field == "artwork");
    bool isRating = (field == "rating");
    bool isUrl = (field == "url" || field == "relatedTo");

    //Set basic formatting info
    QRect dataRect = fieldDataRect(option, index);
    QFont textFont = KGlobalSettings::smallestReadableFont();
    Qt::AlignmentFlag hAlign = Qt::AlignLeft;
    bool showFieldName = true;

    //Formatting modifications based on field info
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
        showFieldName = false;
    } else if (field == "title") {
        showFieldName = false;
        textFont = option.font;
        textFont.setPointSize(1.5*textFont.pointSize());
        hAlign = Qt::AlignHCenter;
    } else if (field == "description") {
        showFieldName = false;
        hAlign = Qt::AlignJustify;
        if (text.isEmpty()) {
            foregroundColor.setAlphaF(0.7);
            text = i18n("No description");
            hAlign = Qt::AlignCenter;
            textFont.setItalic(true);
        }
    } else if (field == "url") {
        textFont.setBold(modified);
        text = QFontMetrics(textFont).elidedText(text, Qt::ElideMiddle, dataRect.width());
    }
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

    //Paint field name
    if (showFieldName) {
        QString fieldName = index.data(InfoItemModel::FieldNameRole).toString();
        QFont fieldNameFont = KGlobalSettings::smallestReadableFont();
        fieldNameFont.setItalic(true);
        int fieldNameWidth = width - dataRect.width() - 3*m_padding;
        QTextOption textOption(Qt::AlignRight | Qt::AlignTop);
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        QRect fieldNameRect(dataRect.left()-fieldNameWidth-2*m_padding, dataRect.top(), fieldNameWidth, dataRect.height());
        p.setFont(fieldNameFont);
        p.setPen(foregroundColor);
        p.drawText(QRectF(fieldNameRect), fieldName, textOption);
    }

    //Paint field data
    if (isArtwork) {
        //Paint Artwork
        if (isEditable &&
            option.state.testFlag(QStyle::State_MouseOver) &&
            !m_isEditing) {
            //Draw hover rectangle
            p.save();
            QRect hoverRect(dataRect.adjusted(-m_padding, -m_padding, m_padding, m_padding));
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(hoverColor));
            p.drawRoundedRect(hoverRect, 3.0, 3.0);
            p.restore();
        }
        QList<QVariant> artworkList = index.data(InfoItemModel::ArtworkListRole).toList();
        ArtworkPainter artworkPainter;
        if (artworkList.count() == 0) {
            QIcon artwork = index.data(Qt::DecorationRole).value<QIcon>();
            artworkPainter.paint(&p, dataRect, artwork.pixmap(128, 128));
        } else {
            artworkPainter.paint(&p, dataRect, artworkList);
        }

        if (isEditable &&
            option.state.testFlag(QStyle::State_MouseOver)) {
            QString artworkUrl = index.data(Qt::EditRole).toString();
            //Draw clear field "button"
            if (!artworkUrl.isEmpty()) {
                int clrTop = dataRect.top() + (dataRect.height() - 16)/2;
                int clrLeft = dataRect.right() - 16;
                KIcon("edit-clear-locationbar-rtl").paint(&p, clrLeft, clrTop, 16, 16);
            }
        }
    } else if (isRating) {
        //Paint rating
        int rating = index.data(Qt::DisplayRole).toInt();
        StarRating starRating = StarRating(rating, m_starRatingSize);
        starRating.setRating(rating);
        QSize ratingSize = starRating.sizeHint();
        int ratingLeft = dataRect.left() + (dataRect.width()-ratingSize.width())/2;
        int ratingTop = dataRect.top() + (dataRect.height() - ratingSize.height())/2;
        QRect ratingRect = QRect(QPoint(ratingLeft, ratingTop), ratingSize);
        starRating.setPoint(ratingRect.topLeft());
        if (option.state.testFlag(QStyle::State_MouseOver) && !m_application->isTouchEnabled()) {
            starRating.setHoverAtPosition(m_mousePos);
        }
        starRating.paint(&p);
    } else {
        QTextOption textOption(hAlign | Qt::AlignTop);
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        if (index.data(Qt::DisplayRole).type() == QVariant::StringList) {
            //Render field data in a list
            QStringList textList = index.data(Qt::DisplayRole).toStringList();
            QStringList originalTextList = index.data(InfoItemModel::OriginalValueRole).toStringList();
            QList<QVariant> drillList = index.data(InfoItemModel::DrillRole).toList();
            int textHeight = QFontMetrics(textFont).height();
            for (int i = 0; i <= textList.count(); i++) {
                if (i == textList.count() && i > 0) {
                    break;
                }
                int top = dataRect.top()+i*(textHeight+2*m_padding);
                QRect textRect(dataRect.left(), top, dataRect.width(), textHeight);
                QRect hoverRect = textRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);
                if (isEditable &&
                    option.state.testFlag(QStyle::State_MouseOver) && hoverRect.contains(m_mousePos) &&
                    !m_isEditing) {
                    //Draw hover rectangle
                    p.save();
                    p.setPen(Qt::NoPen);
                    p.setBrush(QBrush(hoverColor));
                    p.drawRoundedRect(hoverRect, 3.0, 3.0);
                    p.restore();

                    //Draw link icon
                    QRect linkIconRect;
                    if (isUrl && !textList.isEmpty() && option.state.testFlag(QStyle::State_MouseOver)) {
                        linkIconRect = textRect.adjusted(textRect.width() - 16, 0, 0, 0);
                        p.save();
                        p.setPen(Qt::NoPen);
                        QColor linkHoverColor = hoverColor;
                        linkHoverColor.setAlpha(255);
                        p.setBrush(QBrush(linkHoverColor));
                        p.drawRoundedRect(linkIconRect.adjusted(0,-m_padding, m_padding, m_padding), 3.0, 3.0);
                        p.drawRect(linkIconRect.adjusted(0,-m_padding, 0, m_padding));
                        p.restore();
                        KIcon("emblem-symbolic-link").paint(&p, linkIconRect.adjusted(0,-m_padding, m_padding, m_padding));
                        textRect.adjust(0, 0, -linkIconRect.width(), 0);
                    }

                    //Draw drill icon
                    QRect drillIconRect;
                    if (i < drillList.count()) {
                        if (!drillList.at(i).isNull()) {
                            drillIconRect = textRect.adjusted(textRect.width()-16, 0, 0, 0);
                        }
                    }
                    if (!drillIconRect.isNull()) {
                        p.save();
                        p.setPen(Qt::NoPen);
                        QColor drillHoverColor = hoverColor;
                        drillHoverColor.setAlpha(255);
                        p.setBrush(QBrush(drillHoverColor));
                        p.drawRoundedRect(drillIconRect.adjusted(0,-m_padding, m_padding, m_padding), 3.0, 3.0);
                        p.drawRect(drillIconRect.adjusted(0,-m_padding, 0, m_padding));
                        p.restore();
                        if (drillIconRect.contains(m_mousePos)) {
                            m_drillIconHighlight.paint(&p, drillIconRect.adjusted(0,-m_padding, m_padding, m_padding));
                        } else {
                            m_drillIcon.paint(&p, drillIconRect.adjusted(0,-m_padding, m_padding, m_padding));
                        }
                        textRect.adjust(0, 0, -drillIconRect.width(), 0);
                    }
                    //Draw plus icon
                    QRect plusIconRect;
                    if (i == textList.count()-1) {
                        plusIconRect = textRect.adjusted(textRect.width()-16, 0, 0, 0);
                    }
                    if (!plusIconRect.isNull()) {
                        p.save();
                        p.setPen(Qt::NoPen);
                        QColor plusHoverColor = hoverColor;
                        plusHoverColor.setAlpha(255);
                        p.setBrush(QBrush(plusHoverColor));
                        if (drillIconRect.isNull()) {
                            p.drawRoundedRect(plusIconRect.adjusted(0,-m_padding, m_padding, m_padding), 3.0, 3.0);
                        }
                        p.drawRect(plusIconRect.adjusted(0,-m_padding, 0, m_padding));
                        int addTop = plusIconRect.top() + (plusIconRect.height()-6)/2;
                        int addLeft = plusIconRect.left() + (plusIconRect.width()-6)/2 + 1;
                        QColor color = option.palette.color(QPalette::HighlightedText);
                        if (plusIconRect.contains(m_mousePos)) {
                            color.setAlpha(255);
                        } else {
                            color.setAlpha(200);
                        }
                        QPen pen(color);
                        pen.setWidth(2);
                        p.setPen(pen);
                        p.drawLine(addLeft+3, addTop, addLeft+3, addTop+6);
                        p.drawLine(addLeft, addTop+3, addLeft+6, addTop+3);
                        p.restore();
                        textRect.adjust(0, 0, -plusIconRect.width(), 0);
                    }
                }

                //Paint field list data
                if (i < textList.count()) {
                    if (i >= originalTextList.count())  {
                        textFont.setBold(true);
                    } else if (textList.at(i) == originalTextList.at(i)) {
                        textFont.setBold(false);
                    } else {
                        textFont.setBold(true);
                    }
                    if (!isUrl) {
                        text = QFontMetrics(textFont).elidedText(textList.at(i), Qt::ElideRight, textRect.width());
                    } else {
                        KUrl url(textList.at(i));
                        if (!url.isLocalFile()) {
                            text = QFontMetrics(textFont).elidedText(url.host(), Qt::ElideMiddle, textRect.width());
                        } else {
                            text = QFontMetrics(textFont).elidedText(textList.at(i), Qt::ElideMiddle, textRect.width());
                        }
                    }
                    p.setFont(textFont);
                    p.setPen(foregroundColor);
                    p.drawText(QRectF(textRect), text, textOption);
                } else if (multipleValues) {
                    p.setFont(textFont);
                    p.setPen(foregroundColor);
                    p.drawText(QRectF(textRect), text, textOption);
                }
            }
        } else {
            //Render field data
            QRect textRect = dataRect;
            QRect hoverRect = dataRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);
            if (isEditable &&
                option.state.testFlag(QStyle::State_MouseOver) &&
                hoverRect.contains(m_mousePos) &&
                !m_isEditing) {
                //Draw hover rectangle
                p.save();
                p.setPen(Qt::NoPen);
                p.setBrush(QBrush(hoverColor));
                p.drawRoundedRect(hoverRect, 3.0, 3.0);
                p.restore();

                //Draw drill icon
                QVariant drillItem = index.data(InfoItemModel::DrillRole);
                QRect drillIconRect;
                if (!drillItem.isNull()) {
                    drillIconRect = textRect.adjusted(textRect.width() - 16, 0, 0, 0);
                }
                if (!drillIconRect.isNull()) {
                    p.save();
                    p.setPen(Qt::NoPen);
                    QColor drillHoverColor = hoverColor;
                    drillHoverColor.setAlpha(255);
                    p.setBrush(QBrush(drillHoverColor));
                    p.drawRoundedRect(drillIconRect.adjusted(0,-m_padding, m_padding, m_padding), 3.0, 3.0);
                    p.drawRect(drillIconRect.adjusted(0,-m_padding, 0, m_padding));
                    p.restore();
                    if (drillIconRect.contains(m_mousePos)) {
                        m_drillIconHighlight.paint(&p, drillIconRect.adjusted(0,-m_padding, m_padding, m_padding));
                    } else {
                        m_drillIcon.paint(&p, drillIconRect.adjusted(0,-m_padding, m_padding, m_padding));
                    }
                    textRect.adjust(0, 0, -drillIconRect.width(), 0);
                }

            }

            //Draw link icon
            if (isUrl && option.state.testFlag(QStyle::State_MouseOver) && !m_application->isTouchEnabled()) {
                KUrl url(index.data().toString());
                if (url.isValid() &&
                    (field == "relatedTo" ||
                     (field == "url" && url.isLocalFile()))) {
                    QRect linkIconRect = textRect.adjusted(textRect.width() - 16, 0, 0, 0);
                    KIcon("emblem-symbolic-link").paint(&p, linkIconRect.adjusted(0,-m_padding, m_padding, m_padding));
                    textRect.adjust(0, 0, -linkIconRect.width(), 0);
                    text = QFontMetrics(textFont).elidedText(text, Qt::ElideMiddle, textRect.width());
                }
            }

            //Draw field data
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
    Q_UNUSED(option);
    return QSize(0, rowHeight(index.row()));
}

bool InfoItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    BangarangApplication *application = (BangarangApplication *)KApplication::kApplication();

    m_mousePos = ((QMouseEvent *)event)->pos();
    if (!m_isEditing) {
        m_view->update(index);
    }
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (field == "artwork") {
        bool isEditable = ((QStandardItemModel *)model)->itemFromIndex(index)->isEditable() && !m_suppressEditing;
        if (event->type() == QEvent::MouseButtonRelease && isEditable) {
            QRect clearButtonRect = QRect(option.rect.left()+option.rect.width()-16, option.rect.top()+(option.rect.height()-16)/2, 16, 16);
            if (clearButtonRect.contains(m_mousePos)) {
                //Clear set artwork
                InfoItemModel *model = (InfoItemModel *)index.model();
                model->clearArtwork();
            } else {
                //Get artwork url from user
                QString artworkUrl = index.data(Qt::EditRole).toString();
                if (artworkUrl.isEmpty()) {
                    InfoItemModel *model = (InfoItemModel *)index.model();
                    if (model->mediaList().count() > 0) {
                        artworkUrl = KUrl(model->mediaList().at(0).url).directory();
                    }
                }
                KUrl newUrl = KFileDialog::getImageOpenUrl(KUrl(artworkUrl), application->mainWindow(), i18n("Open artwork file"));
                if (newUrl.isValid()) {
                    model->setData(index, newUrl.url(), Qt::EditRole);
                    model->setData(index, false, InfoItemModel::MultipleValuesRole);
                }
                application->mainWindow()->ui->mediaView->setFocus();
            }
        }
        return true;
    } else if (field == "rating") {
        if (event->type() == QEvent::MouseButtonRelease) {
            //Determine rating set a mouse position
            QRect dataRect = fieldDataRect(option, index);
            QSize ratingSize = StarRating::SizeHint(m_starRatingSize);
            int ratingLeft = dataRect.left() + (dataRect.width()-ratingSize.width())/2;
            int ratingTop = dataRect.top() + (dataRect.height() - ratingSize.height())/2;
            QRect ratingRect = QRect(QPoint(ratingLeft, ratingTop), ratingSize);
            int rating = StarRating::RatingAtPosition(m_mousePos, m_starRatingSize, ratingRect.topLeft());

            //Set rating
            InfoItemModel *model = (InfoItemModel *)index.model();
            model->setRating(rating);
            QList<MediaItem> mediaList = model->mediaList();
            QList<MediaItemModel *> modelsToUpdate;
            modelsToUpdate.append(application->playlist()->playlistModel());
            modelsToUpdate.append(application->playlist()->queueModel());
            modelsToUpdate.append(application->playlist()->nowPlayingModel());
            modelsToUpdate.append(application->browsingModel());
            for (int i = 0; i < modelsToUpdate.count(); i++) {
                MediaItemModel *modelToUpdate = modelsToUpdate.at(i);
                for (int j = 0; j < mediaList.count(); j++) {
                    int row = modelToUpdate->rowOfUrl(mediaList.at(j).url);
                    if (row != -1) {
                        MediaItem mediaItem = modelToUpdate->mediaItemAt(row);
                        mediaItem.fields["rating"] = rating;
                        modelToUpdate->replaceMediaItemAt(row, mediaItem);
                    }
                }
            }
        }
        return true;
    }  else if (field == "url") {
        if (event->type() == QEvent::MouseButtonPress) {
            bool isEditable = ((QStandardItemModel *)model)->itemFromIndex(index)->isEditable() && !m_suppressEditing;
            QRect dataRect = fieldDataRect(option, index);
            QRect hoverRect = dataRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);
            KUrl url(index.data(Qt::DisplayRole).toString());
            url = KUrl(url.directory());
            if (url.isValid() && url.isLocalFile()) {
                QRect linkIconRect = dataRect.adjusted(dataRect.width() - 16, -m_padding, m_padding, m_padding);
                hoverRect.adjust(0, 0, -16 - m_padding, 0);
                if (linkIconRect.contains(m_mousePos) ||
                    (m_application->isTouchEnabled() && hoverRect.contains(m_mousePos) & !isEditable)) {
                    QDesktopServices::openUrl(url);
                }
            }
            if (hoverRect.contains(m_mousePos)) {
                m_isEditing = true;
                return QItemDelegate::editorEvent(event, model, option, index);
            }
        }
        return true;
    } else {
        //Determine where mouse button was pressed and handle approriate
        int listIndex = stringListIndexAtMousePos(option, index);
        QRect dataRect = fieldDataRect(option, index);
        QRect hoverRect = dataRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);

        bool linkIconExists = (field == "relatedTo");
        bool drillIconExists = false;
        if (index.data(Qt::DisplayRole).type() == QVariant::String) {
            QVariant drillItem = index.data(InfoItemModel::DrillRole);
            if (!drillItem.isNull()) {
                drillIconExists = true;
            }
        }
        if (index.data(Qt::DisplayRole).type() == QVariant::StringList ) {
            QList<QVariant> drillList = index.data(InfoItemModel::DrillRole).toList();
            if (listIndex != -1 && listIndex < drillList.count()) {
                if (!drillList.at(listIndex).isNull()) {
                    drillIconExists = true;
                }
            }
        }

        QRect linkIconRect;
        if (linkIconExists) {
            linkIconRect = dataRect.adjusted(dataRect.width() - 16, -m_padding, m_padding, m_padding);
            hoverRect.adjust(0, 0, -16 - m_padding, 0);
            dataRect.adjust(0, 0, -16, 0);
        }
        QRect drillIconRect;
        if (drillIconExists) {
            drillIconRect = dataRect.adjusted(dataRect.width() - 16, -m_padding, m_padding, m_padding);
            hoverRect.adjust(0, 0, -16 - m_padding, 0);
            dataRect.adjust(0, 0, -16, 0);
        }
        QRect plusIconRect;
        QStringList textList = index.data(Qt::DisplayRole).toStringList();
        if (index.data(Qt::DisplayRole).type() == QVariant::StringList &&
            listIndex == textList.count()-1) {
            plusIconRect = dataRect.adjusted(dataRect.width()-16, 0, 0, 0);
        }
        if (hoverRect.contains(m_mousePos) && !m_isEditing && !m_suppressEditing) {
            if (event->type() == QEvent::MouseButtonRelease) {
                m_isEditing = true;
            }
            if (plusIconRect.contains(m_mousePos)) {
                if (index.data(Qt::DisplayRole).type() == QVariant::StringList && event->type() == QEvent::MouseButtonPress) {
                    //Editing new value
                    m_stringListIndexEditing = -1;
                    m_rowOfNewValue = index.row();
                    emit sizeHintChanged(index);
//                     QApplication::processEvents(); //this would disturb the flow and cause a wrong line edit to exist
                    m_view->update(index);
                    m_view->fixHeightToContents();
                }
            } else {
                if (index.data(Qt::DisplayRole).type() == QVariant::StringList && event->type() == QEvent::MouseButtonPress) {
                    //Editing existing value
                    m_stringListIndexEditing = stringListIndexAtMousePos(option, index);
                    m_rowOfNewValue = -1;
                }
            }
            return QItemDelegate::editorEvent(event, model, option, index);
        } else if (drillIconRect.contains(m_mousePos) && !m_isEditing && !m_application->isTouchEnabled()) {
            if (event->type() == QEvent::MouseButtonRelease) {
                m_rowOfNewValue = -1;
                MediaItem drillItem = index.data(InfoItemModel::DrillRole).value<MediaItem>();
                if (index.data(Qt::DisplayRole).type() == QVariant::StringList) {
                    QList<QVariant> drillList = index.data(InfoItemModel::DrillRole).toList();
                    if (listIndex != -1 && listIndex < drillList.count()) {
                        drillItem = drillList.at(listIndex).value<MediaItem>();
                    }
                }
                if (!drillItem.url.isEmpty()) {
                    MediaListProperties mediaListProperties;
                    mediaListProperties.lri = drillItem.url;
                    mediaListProperties.name = drillItem.title;
                    mediaListProperties.category = drillItem;
                    application->mediaListsManager()->addListToHistory();
                    application->browsingModel()->clearMediaListData();
                    application->browsingModel()->setMediaListProperties(mediaListProperties);
                    application->browsingModel()->load();
                }
                return true;
            } else {
                return true;
            }
        } else if (linkIconRect.contains(m_mousePos) |
                   (linkIconExists && hoverRect.contains(m_mousePos) && m_application->isTouchEnabled())) {
            if (event->type() == QEvent::MouseButtonRelease) {
                KUrl url;
                if (index.data(Qt::DisplayRole).type() == QVariant::StringList) {
                    QList<QVariant> urlList = index.data().toList();
                    if (listIndex != -1 && listIndex < urlList.count()) {
                        url = urlList.at(listIndex).toString();
                    }
                } else {
                    url = KUrl(index.data(Qt::DisplayRole).toString());
                }
                if (url.isValid()) {
                    QDesktopServices::openUrl(url);
                }
            }
            return true;
        } else {
            return true;
        }
    }
}

QWidget *InfoItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QVariant value = index.data(Qt::EditRole);
    if (field == "artwork" || field == "rating") {
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
            connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
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
            connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
            return comboBox;
        } else {
            KLineEdit *lineEdit = new KLineEdit(parent);
            lineEdit->setInputMask("0000");
            lineEdit->setFont(KGlobalSettings::smallestReadableFont());
            lineEdit->setAutoFillBackground(true);
            return lineEdit;
        }
    } else if (value.type() == QVariant::StringList) {
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        if (valueList.count() == 0) {
            KLineEdit *lineEdit = new KLineEdit(parent);
            lineEdit->setFont(KGlobalSettings::smallestReadableFont());
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
    } else if (value.type() == QVariant::String) {
        if (field == "description") {
            QTextEdit *textEdit = new QTextEdit(parent);
            textEdit->setAcceptRichText(false);
            textEdit->setFont(KGlobalSettings::smallestReadableFont());
            textEdit->setAutoFillBackground(true);
            textEdit->setToolTip(i18n("Press <Tab> to finish editing."));
            textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            return textEdit;
        }
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        if (valueList.count() == 0) {
            KLineEdit *lineEdit = new KLineEdit(parent);
            lineEdit->setFont(KGlobalSettings::smallestReadableFont());
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
    } else if (field != "artwork") {
            KLineEdit *lineEdit = new KLineEdit(parent);
            lineEdit->setFont(KGlobalSettings::smallestReadableFont());
            lineEdit->setAutoFillBackground(true);
            return lineEdit;
    } else {
        return 0;
    }
    Q_UNUSED(option);

}

void InfoItemDelegate::setEditorData (QWidget * editor, const QModelIndex & index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QVariant::Type type = index.data(Qt::EditRole).type();
    bool multipleValues = index.data(InfoItemModel::MultipleValuesRole).toBool();
    if (type == QVariant::String) {
        if (multipleValues) {
            return;
        }
        QString text = index.data(Qt::EditRole).toString();
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        if (valueList.isEmpty()) {
            if (field == "description") {
                QTextEdit *textEdit = qobject_cast<QTextEdit*>(editor);
                textEdit->setText(text);
            } else {
                KLineEdit *lineEdit = qobject_cast<KLineEdit*>(editor);
                lineEdit->setText(text);
            }
        } else {
            SComboBox *comboBox = qobject_cast<SComboBox*>(editor);
            comboBox->lineEdit()->setText(text);
        }
        return;
    }
    if (type == QVariant::StringList) {
        if (multipleValues) {
            return;
        }
        QString text;
        QStringList textList = index.data(Qt::EditRole).toStringList();
        if (m_stringListIndexEditing != -1 && m_stringListIndexEditing < textList.count()) {
            text = textList.at(m_stringListIndexEditing);
        }
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        if (valueList.isEmpty()) {
            KLineEdit *lineEdit = qobject_cast<KLineEdit*>(editor);
            lineEdit->setText(text);
        } else {
            SComboBox *comboBox = qobject_cast<SComboBox*>(editor);
            comboBox->lineEdit()->setText(text);
        }
        return;
    }
    if (index.data(Qt::EditRole).type() == QVariant::Int) {
	if (multipleValues)
	    return;
	QVariant data = index.data(Qt::EditRole);
	if (data.isNull())
	    return;
        if (field == "audioType" || field == "videoType") {
            QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
	    comboBox->setCurrentIndex(data.toInt());
	    return;
	}
	KLineEdit *lineEdit = qobject_cast<KLineEdit*>(editor);
	lineEdit->setText(data.toString());
	return;
    }
    QItemDelegate::setEditorData(editor, index);
}

void InfoItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (index.data(Qt::DisplayRole).type() == QVariant::StringList) {
        QRect editorRect = stringListRectAtMousePos(option, index);
        editor->setGeometry(editorRect);
    } else {
        QRect dataRect = fieldDataRect(option, index);
        QRect editorRect = dataRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);
        editor->setGeometry(editorRect);
    }
}

void InfoItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const
{
    QString field = index.data(InfoItemModel::FieldRole).toString();
    if (field == "audioType" || field == "videoType") {
        QComboBox * comboBox = qobject_cast<QComboBox*>(editor);
        if (!comboBox) {
            return;
        }
        model->setData(index, comboBox->currentIndex(), Qt::EditRole);
    } else if (field == "description") {
        QTextEdit * textEdit = qobject_cast<QTextEdit*>(editor);
        if (!textEdit) {
            return;
        }
        model->setData(index, textEdit->toPlainText(), Qt::DisplayRole);
        model->setData(index, textEdit->toPlainText(), Qt::EditRole);
    } else if (index.data(Qt::EditRole).type() == QVariant::String){
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        QString text;
        if (valueList.isEmpty()) {
            KLineEdit * lineEdit = qobject_cast<KLineEdit*>(editor);
            if (!lineEdit) {
                return;
            }
            text = lineEdit->text().trimmed();
        } else {
            SComboBox * comboBox = qobject_cast<SComboBox*>(editor);
            if (!comboBox) {
                return;
            }
            text = comboBox->lineEdit()->text().trimmed();
        }
        model->setData(index, text, Qt::DisplayRole);
        model->setData(index, text, Qt::EditRole);
    } else if (index.data(Qt::EditRole).type() == QVariant::StringList){
        QStringList valueList = index.data(InfoItemModel::ValueListRole).toStringList();
        QString text;
        if (valueList.isEmpty()) {
            KLineEdit * lineEdit = qobject_cast<KLineEdit*>(editor);
            if (!lineEdit) {
                return;
            }
            text = lineEdit->text().trimmed();
        } else {
            SComboBox * comboBox = qobject_cast<SComboBox*>(editor);
            if (!comboBox) {
                return;
            }
            text = comboBox->lineEdit()->text().trimmed();
        }
        QStringList textList = index.data(Qt::EditRole).toStringList();
        if (m_stringListIndexEditing == -1 && !text.isEmpty()){
            textList.append(text);
        } else if (m_stringListIndexEditing >= 0 && !text.isEmpty()) {
            textList.replace(m_stringListIndexEditing, text);
        } else if (m_stringListIndexEditing >= 0 && text.isEmpty()) {
            textList.removeAt(m_stringListIndexEditing);
        } else {
            return;
        }
        model->setData(index, textList, Qt::DisplayRole);
        model->setData(index, textList, Qt::EditRole);
    } else if (index.data(Qt::EditRole).type() == QVariant::Int) {
        KLineEdit * lineEdit = qobject_cast<KLineEdit*>(editor);
        if (!lineEdit) {
            return;
        }
        if (!lineEdit->text().trimmed().isEmpty()) {
            int value = lineEdit->text().toInt();
            model->setData(index, value, Qt::DisplayRole);
            model->setData(index, value, Qt::EditRole);
        } else {
            model->setData(index, QVariant(QVariant::Int), Qt::DisplayRole);
            model->setData(index, QVariant(QVariant::Int), Qt::EditRole);
        }
    } else {
        QItemDelegate::setModelData(editor, model, index);
    }
    
}   

void InfoItemDelegate::setView(InfoItemView * view)
{
    m_view = view;
    m_defaultViewSelectionMode = view->selectionMode();
}

int InfoItemDelegate::heightForWordWrap(QFont font, int width, QString text) const
{
    QFontMetrics fm(font);
    QRect rect(0, 0, width, fm.lineSpacing());
    int height = fm.boundingRect(rect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,text).height();
    return height;
}

int InfoItemDelegate::rowHeight(int row) const
{
    QModelIndex index = m_view->model()->index(row,0);
    QString field = index.data(InfoItemModel::FieldRole).toString();
    QVariant::Type fieldType = index.data(Qt::DisplayRole).type();
    bool modified = (index.data(Qt::DisplayRole) != index.data(InfoItemModel::OriginalValueRole));
    int width = m_view->width();

    int height;
    if (field == "artwork") {
        height = 128 + 10 + 2*m_padding ; //10 pixel to accomodate rotated artwork
    } else if (field == "rating") {
        height = StarRating::SizeHint(m_starRatingSize).height() + 2 *m_padding;
    } else {
        QString text = index.data(Qt::DisplayRole).toString();
        if (fieldType == QVariant::StringList) {
            QStringList textList = index.data(Qt::DisplayRole).toStringList();
            if (textList.count() > 0) {
                text = textList.at(0);
            }
        }
        QFont textFont = KGlobalSettings::smallestReadableFont();
        int fieldNameWidth = qMax(70, (width - 4 * m_padding)/4);
        int availableWidth = width - 5 * m_padding - fieldNameWidth;
        if (field == "title") {
            textFont = QFont();
            textFont.setPointSize(1.5*textFont.pointSize());
            availableWidth = width - 4*m_padding;
        } else if (field == "description") {
            availableWidth = width - 4*m_padding;
        } else if (field == "url") {
            text = QString(" "); // url text is elided to a single line anyway
        }
        if (availableWidth <= 0) {
            availableWidth = 100;
        }
        if (modified) {
            textFont.setBold(true);
        }
        height = heightForWordWrap(textFont, availableWidth, text) + 2*m_padding;
        if (fieldType == QVariant::StringList) {
            QStringList textList = index.data(Qt::DisplayRole).toStringList();
            int rows = textList.count();
            if (m_rowOfNewValue == row || rows == 0){
               rows++;
            }
            height = rows*height;
        }

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

QRect InfoItemDelegate::fieldDataRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //Get basic information about painting area
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = option.rect.height();

    //Get basic information about field
    QString field = index.data(InfoItemModel::FieldRole).toString();

    //Set basic formatting info
    int padding = m_padding;
    int fieldNameWidth = qMax(70, (width - 4 * padding)/4);
    int textWidth = width - 5 * padding - fieldNameWidth;
    int textLeft = left + fieldNameWidth + 3 * padding;

    //Special handling for some field types
    if (field == "artwork" ||
        field == "title" ||
        field == "rating" ||
        field == "description") {
        textWidth = width - 4 * padding;
        textLeft = left + 2*padding;
    }

    return QRect(textLeft, top+padding, textWidth, height-2*padding);

}

int InfoItemDelegate::stringListIndexAtMousePos(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int foundIndex = -1;
    if (index.data(Qt::DisplayRole).type() == QVariant::StringList) {
        QRect dataRect = fieldDataRect(option, index);
        QStringList textList = index.data(Qt::DisplayRole).toStringList();
        if (textList.count() != 0) {
            int textHeight = QFontMetrics(KGlobalSettings::smallestReadableFont()).height();
            for (int i = 0; i < textList.count(); i++) {
                QRect textRect(dataRect.left(), dataRect.top()+i*(textHeight+2*m_padding), dataRect.width(), textHeight);
                QRect hoverRect(textRect.adjusted(-m_padding, -m_padding, m_padding, m_padding));
                if (hoverRect.contains(m_mousePos)) {
                    foundIndex = i;
                    break;
                }
            }
        }
    }
    return foundIndex;
}

QRect InfoItemDelegate::stringListRectAtMousePos(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect foundRect;
    if (index.data(Qt::DisplayRole).type() == QVariant::StringList) {
        QRect dataRect = fieldDataRect(option, index);
        QStringList textList = index.data(Qt::DisplayRole).toStringList();
        if (textList.count() == 0) {
            QRect hoverRect = dataRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);
            if (hoverRect.contains(m_mousePos)) {
                foundRect = hoverRect;
            }
        } else {
            int textHeight = QFontMetrics(KGlobalSettings::smallestReadableFont()).height();
            for (int i = 0; i < textList.count(); i++) {
                int top = dataRect.top()+i*(textHeight+2*m_padding);
                QRect textRect(dataRect.left(), top, dataRect.width(), textHeight);
                QRect hoverRect = textRect.adjusted(-m_padding, -m_padding, m_padding, m_padding);
                if (hoverRect.contains(m_mousePos)) {
                    QString field = index.data(InfoItemModel::FieldRole).toString();
                    if (field == "url" || field == "relatedTo") { //adjust for presence of icon rect
                        textRect.adjust(0, 0, -16, 0);
                    }
                    QList<QVariant> drillLriList = index.data(InfoItemModel::DrillRole).toList();
                    QRect drillIconRect;
                    if (i < drillLriList.count()) {
                        if (!drillLriList.at(i).isNull()) {
                            drillIconRect = textRect.adjusted(textRect.width()-16, -m_padding, m_padding, m_padding);
                            textRect.adjust(0,0,-16,0);
                        }
                    }
                    QRect plusIconRect;
                    if (i == textList.count() - 1) {
                        plusIconRect = textRect.adjusted(textRect.width()-16, -m_padding, 0, m_padding);
                    }
                    if (plusIconRect.contains(m_mousePos)) {
                        foundRect = hoverRect.adjusted(0, textHeight+2*m_padding, 0, textHeight+2*m_padding);
                    } else {
                        foundRect = hoverRect;
                    }
                    break;
                }
            }
        }
    }
    return foundRect;
}

void InfoItemDelegate::endEditing(QWidget * editor)
{
    resetEditMode();
    Q_UNUSED(editor);
}

void InfoItemDelegate::resetEditMode()
{
    m_isEditing = false;
    if (m_rowOfNewValue != -1) {
        InfoItemModel * model = (InfoItemModel *)m_view->model();
        QModelIndex index = model->index(m_rowOfNewValue,0);
        emit sizeHintChanged(index);
        m_rowOfNewValue = -1;
    }
}

void InfoItemDelegate::enableTouch() {
    m_starRatingSize = StarRating::Huge;
}

void InfoItemDelegate::typeChanged(int i)
{
    m_typeChanged = true;
    Q_UNUSED(i);
}

bool InfoItemDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (m_typeChanged) {
        m_typeChanged = false;
        QKeyEvent *enterEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        return QItemDelegate::eventFilter(editor, enterEvent);
    } else {
        return QItemDelegate::eventFilter(editor, event);
    }
}

void InfoItemDelegate::suppressEditing(bool suppress)
{
    m_suppressEditing = suppress;
}
