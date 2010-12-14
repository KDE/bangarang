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

#include "nowplayingdelegate.h"
#include "bangarangapplication.h"
#include "infomanager.h"
#include "platform/utilities/utilities.h"
#include "platform/mediaitemmodel.h"
#include "platform/mediaindexer.h"
#include "platform/mediavocabulary.h"
#include "platform/playlist.h"

#include <QPalette>
#include <QStyle>
#include <QIcon>
#include <QFont>
#include <QStandardItem>
#include <QApplication>
#include <KGlobalSettings>
#include <KColorScheme>
#include <KIcon>
#include <KIconEffect>
#include <QTextOption>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include "mainwindow.h"
#include "starrating.h"
#include <ui_mainwindow.h>

NowPlayingDelegate::NowPlayingDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_parent = (MainWindow *)parent;
    m_globalRatingRect = QRect(0, 0, 0, 0);
    m_view = NULL;
    m_iconSize = 128;
    m_padding = 6;
    m_textInner = m_iconSize + 2 * m_padding;
    m_starRatingSize = StarRating::Big;
    m_showInfo = false;

    m_nepomukInited = Utilities::nepomukInited();
    if (m_nepomukInited) {
        m_mediaIndexer = new MediaIndexer(this);
    }
}

NowPlayingDelegate::~NowPlayingDelegate()
{
}

void NowPlayingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() != 0 && index.column() !=0) {
        return;
    }
    
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = option.rect.height();

    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected)) ?
        option.palette.color(QPalette::HighlightedText) :
        option.palette.color(QPalette::Text);

    bool isMedia = Utilities::isMediaItem(&index);
    QString type = index.data(MediaItem::TypeRole).toString();
    MediaItemModel * model = (MediaItemModel *)index.model();
    QString subType;
    if (type == "Audio") {
        subType = model->mediaItemAt(index.row()).fields["audioType"].toString();
    } else if (type == "Video") {
        subType = model->mediaItemAt(index.row()).fields["videoType"].toString();
    }

    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());
    
    //Paint Artwork
    KIcon icon(index.data(Qt::DecorationRole).value<QIcon>());
    int topOffset = (height - 2*m_iconSize) / 2;
    
    if (!icon.isNull() && m_iconSize > 0) {
        icon.paint(&p, left + m_padding, top + topOffset, m_iconSize, m_iconSize,
                    Qt::AlignCenter, QIcon::Normal);
    }

    //Paint text
    QString text = index.data(Qt::DisplayRole).toString();
    QFont textFont = option.font;
    QTextOption textOption(Qt::AlignLeft | Qt::AlignBottom);
    textOption.setWrapMode(QTextOption::WordWrap);
    QRect titleRect(left + m_textInner,
                    top + m_padding, width - m_textInner - m_padding, 0.4 * m_iconSize + topOffset);
    textFont.setPixelSize((int)(m_iconSize/6));
    p.setFont(textFont);
    p.setPen(foregroundColor);
    p.drawText(QRectF(titleRect), text, textOption);
    
    QString subTitle = index.data(MediaItem::SubTitleRole).toString();
    
    
    QFont subTitleFont = option.font;
    QRect subTitleRect(left + m_textInner,
                       top + topOffset + 0.4 * m_iconSize,
                       width - m_textInner - m_padding,
                       0.6 * m_iconSize - 18);
    subTitleFont.setPixelSize((int)(m_iconSize/9));
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QColor subTitleColor = KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    p.setFont(subTitleFont);
    p.setPen(subTitleColor);
    p.drawText(QRectF(subTitleRect), subTitle, textOption);

    //Draw rating
    if (m_nepomukInited && isMedia && (subType != "CD Track") && (subType != "DVD Title")) {
        int rating = index.data(MediaItem::RatingRole).isValid() ?
            index.data(MediaItem::RatingRole).toInt() : 0;
        StarRating r = StarRating(rating, m_starRatingSize, ratingRect(&option.rect).topLeft());
        if (option.state.testFlag(QStyle::State_MouseOver))
            r.setHoverAtPosition(m_view->mapFromGlobal(QCursor::pos()));
        r.paint(&p);
    }

    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);
    QPixmap baseReflectionPixmap = pixmap.copy(0,0,pixmap.width(),pixmap.height()/2);
    QPixmap reflection = Utilities::reflection(baseReflectionPixmap);
    painter->drawPixmap(option.rect.topLeft() + QPoint(0, 1+height/2), reflection);

    MediaItem mediaItem = model->mediaItemAt(index.row());
    if (mediaItem.type != "Audio" && mediaItem.type != "Video") {
        return;
    }

    //Draw additional info
    //TODO::The code below could probably use a pass to reduce duplication
    if (!m_showInfo) {
        return;
    }

    QFont infoFont = KGlobalSettings::smallestReadableFont();
    QFontMetrics fm(infoFont);
    QFont fieldFont = KGlobalSettings::smallestReadableFont();
    fieldFont.setBold(true);
    QColor fieldColor = foregroundColor;
    fieldColor.setAlpha(190);
    int infoWidth = option.rect.width() - 20;
    if (m_parent->videoSize() == MainWindow::Mini) {
        infoWidth = infoWidth - m_parent->ui->videoFrame->width() - 20;
    }
    painter->setPen(subTitleColor);
    painter->setFont(infoFont);
    painter->setRenderHint(QPainter::Antialiasing);

    if (mediaItem.subType() == "Music") {
        int line = 0;

        QString artists = mediaItem.fields["artist"].toStringList().join(", ");
        if (!artists.isEmpty()) {
            artists = fm.elidedText(artists, Qt::ElideRight, infoWidth);
            line++;
        }
        QString album = mediaItem.fields["album"].toString();
        if (!album.isEmpty()) {
            album = fm.elidedText(album, Qt::ElideRight, infoWidth);
            line++;
        }
        QString genres = mediaItem.fields["genre"].toStringList().join(", ");
        if (!genres.isEmpty()) {
            genres = fm.elidedText(genres, Qt::ElideRight, infoWidth);
            line++;
        }
        QString year = QString("%1").arg(mediaItem.fields["year"].toInt());
        if (!year.isEmpty()) {
            line++;
        }

        QRect infoRect = option.rect.adjusted(5,
                                              option.rect.height()-line*(fm.lineSpacing()+3) - 5,
                                              -option.rect.width() + infoWidth + 5,
                                              -5);
        if (infoRect.height() > option.rect.height()/2) {
            return;
        }
        line = 0;
        if (!artists.isEmpty()) {
            QString field = i18n("Artist: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, artists);
            line++;
        }
        if (!album.isEmpty()) {
            QString field = i18n("Album: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, album);
            line++;
        }
        if (!genres.isEmpty()) {
            QString field = i18n("Genre: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, genres);
            line++;
        }
        if (!year.isEmpty()) {
            painter->save();
            QString field = i18n("Year: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, year);
            line++;
        }
    }
    if (mediaItem.subType() == "Movie") {
        int line = 0;
        QString year = QString("%1").arg(mediaItem.fields["year"].toInt());
        if (!year.isEmpty()) {
            line++;
        }
        QString actors = mediaItem.fields["actor"].toStringList().join(", ");
        if (!actors.isEmpty()) {
            actors = fm.elidedText(actors, Qt::ElideRight, infoWidth);
            line++;
        }
        QString directors = mediaItem.fields["director"].toStringList().join(", ");
        if (!directors.isEmpty()) {
            directors = fm.elidedText(directors, Qt::ElideRight, infoWidth);
            line++;
        }
        QString description = mediaItem.fields["description"].toString();
        if (!description.isEmpty()) {
            QRect descRect = fm.boundingRect(0, 0, infoWidth, fm.lineSpacing(), Qt::TextWordWrap, description);
            line = line + int((0.5+descRect.height())/fm.lineSpacing());
        }

        QRect infoRect = option.rect.adjusted(5,
                                              option.rect.height()-line*(fm.lineSpacing()+3) - 5,
                                              -option.rect.width() + infoWidth + 5,
                                              -5);
        if (infoRect.height() > option.rect.height()/2) {
            return;
        }
        line = 0;
        if (!year.isEmpty()) {
            QString field = i18n("Year: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, year);
            line++;
        }
        if (!actors.isEmpty()) {
            QString field = i18n("Actor: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, actors);
            line++;
        }
        if (!directors.isEmpty()) {
            QString field = i18n("Director: ");
            QFontMetrics fm(fieldFont);
            painter->save();
            painter->setFont(fieldFont);
            painter->setPen(fieldColor);
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, field);
            painter->restore();
            painter->drawText(infoRect.adjusted(fm.width(field), line*(fm.lineSpacing()+3), 0, 0), Qt::TextSingleLine, directors);
            line++;
        }
        if (!description.isEmpty()) {
            painter->drawText(infoRect.adjusted(0, line*(fm.lineSpacing()+3), 0, 0), Qt::TextWordWrap, description);
            line++;
        }
    }
    
}

QSize NowPlayingDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   int width;
   if (index.column() == 0)  {
       width = option.rect.width();
   } else {
       width = 0;
   }
   
   return QSize(width, m_view->height());
}

int NowPlayingDelegate::columnWidth (int column, int viewWidth) const {
    int width;
    if (column == 0)  {
        width = viewWidth;
    } else {
        width = 0;
    }

    return width;
}

void NowPlayingDelegate::setShowInfo(bool showInfo)
{
    m_showInfo = showInfo;
    m_view->update(m_view->model()->index(0,0));
}

bool NowPlayingDelegate::showingInfo()
{
    return m_showInfo;
}


QRect NowPlayingDelegate::ratingRect(const QRect *rect) const
{
    QSize sz = StarRating::SizeHint(m_starRatingSize);
    QPoint p = QPoint(rect->left() + m_textInner,
                      rect->top() + (rect->height() / 2) - sz.height());
    return QRect(p, sz);
}

bool NowPlayingDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    static bool s_mouseOver = false;
    Q_UNUSED(model);
    if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseMove)
        return false;
    if (index.column() != 0)
        return false;
    if (!Utilities::isMediaItem(&index))
        return false;

    QPoint mousePos = ((QMouseEvent *)event)->pos();
    QRect ratingArea = ratingRect(&option.rect);
    if (!m_nepomukInited) {
        return false;
    }
    if  (!ratingArea.contains(mousePos))
    {
        if (s_mouseOver) { //onLeave effect
            s_mouseOver = false;
            m_view->update();
        }
        return false;
    }
    if (!s_mouseOver) //mouse entered
        s_mouseOver = true;
    if (event->type() == QEvent::MouseMove) //mouse over
    {
        m_view->update();
        return false;
    }
    //else the user clicked, so we have to save the new rating
    int rating = StarRating::RatingAtPosition(mousePos, m_starRatingSize, ratingArea.topLeft());
    MediaItemModel *cmodel = (MediaItemModel *)index.model();
    QString url = cmodel->mediaItemAt(index.row()).url;
    bool indexerUpdated = false;
    //models have to be update
    #define MODELS_TO_BE_UPDATED 4
    MediaItemModel *models[MODELS_TO_BE_UPDATED] = {
        cmodel,
        m_application->playlist()->playlistModel(),
        m_application->playlist()->queueModel(),
        m_application->browsingModel()
    };
    for (int i = 0; i < MODELS_TO_BE_UPDATED; i++)
    {
        cmodel = models[i];
        int row = cmodel->rowOfUrl(url);
        if (row < 0)
            continue;
        MediaItem update = cmodel->mediaItemAt(row);
        update.fields["rating"] = rating;
        cmodel->replaceMediaItemAt(row, update);
        if (!indexerUpdated) {
            m_mediaIndexer->updateRating(update.fields["resourceUri"].toString(),rating);
            indexerUpdated = true;
        }
    }
    m_application->infoManager()->loadSelectedInfo();
    return false;
}

void NowPlayingDelegate::updateSizeHint()
{
    emit sizeHintChanged(m_view->model()->index(0,0));
}
