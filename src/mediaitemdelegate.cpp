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

#include "mediaitemdelegate.h"
#include "bangarangapplication.h"
#include "mainwindow.h"
#include "infomanager.h"
#include "starrating.h"
#include "platform/playlist.h"
#include "platform/mediaindexer.h"
#include "platform/mediaitemmodel.h"
#include "platform/utilities.h"

#include <KGlobalSettings>
#include <KColorScheme>
#include <KIcon>
#include <KIconEffect>
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
#include <QSortFilterProxyModel>
#include <QToolTip>

MediaItemDelegate::MediaItemDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_parent = (MainWindow *)parent;
    setRenderMode(NormalMode);
    m_showPlaying = KIcon("media-playback-start");
    m_showInPlaylist = KIcon("mail-mark-notjunk");
    m_categoryActionIcon = KIcon("bangarang-category-browse");
    QImage image = KIcon("mail-mark-notjunk").pixmap(16,16).toImage();
    KIconEffect::toGray(image, 1.0);
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter pp(&pixmap);
    pp.drawImage(QPoint(0,0), image);
    pp.end();
    m_showNotInPlaylist = KIcon(pixmap);
    m_removeFromPlaylist = KIcon("list-remove");

    m_nepomukInited = Utilities::nepomukInited();
    if (m_nepomukInited) {
        m_mediaIndexer = new MediaIndexer(this);
    }

    //no proxy by default
    m_useProxy = false;
    m_starRatingSize = StarRating::Small;
}

MediaItemDelegate::~MediaItemDelegate()
{
}

void MediaItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);    
    
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    //const int height = calcItemHeight();   
    const int height = option.rect.height();   
    
    if (index.column() != 0)
        return;

    
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    QColor subColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText) :
    KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    QColor nowPlayingColor = option.palette.color(QPalette::Highlight);
    nowPlayingColor.setAlpha(70);
    
    //Determine item type
    QString subType;
    QString type = index.data(MediaItem::TypeRole).toString();
    MediaItemModel * model = (MediaItemModel *) index.model();
    if (useProxy())
        model = (MediaItemModel *)((MediaSortFilterProxyModel *)index.model())->sourceModel();
    if (type == "Audio") {
        subType = model->mediaItemAt(index.row()).fields["audioType"].toString();
    } else if (type == "Video") {
        subType = model->mediaItemAt(index.row()).fields["videoType"].toString();
    }
    bool isMediaItem = Utilities::isMedia(type);
    bool isCategory =  Utilities::isCategory(type);
    bool isAction = type == "Action";
    bool isMessage = type == "Message";
    bool hasUrl = !index.data(MediaItem::UrlRole).toString().isEmpty();
    
    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());
    
    //Paint backgroung for currently playing item
    KIcon icon(index.data(Qt::DecorationRole).value<QIcon>());
    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        MediaItem nowPlayingItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
        if (nowPlayingItem.url == index.data(MediaItem::UrlRole).toString()) {
            icon = m_showPlaying;
            QLinearGradient linearGrad(QPointF(left, top), QPointF(left+width, top));
            linearGrad.setColorAt(0, nowPlayingColor);
            linearGrad.setColorAt(0.7, nowPlayingColor);
            linearGrad.setColorAt(1.0, Qt::transparent);
            QBrush brush(linearGrad);
            p.fillRect(left, top, width, height, brush);
        }
    }
    
    //Paint Icon
    bool exists = index.data(MediaItem::ExistsRole).toBool();
    bool hasCustomArtwork = index.data(MediaItem::HasCustomArtworkRole).toBool();
    int iconSize = hasCustomArtwork ? height - 2: m_iconSize;
    int topOffset = (height - iconSize) / 2;
    if (m_renderMode == NormalMode) {
        if (!icon.isNull()) {
            icon.paint(&p, left + topOffset, top + topOffset, iconSize, iconSize, Qt::AlignCenter, QIcon::Normal);
        }
    }
    if (!exists && m_renderMode == NormalMode) {
        KIcon("emblem-unmounted").paint(&p, left + m_padding, top + topOffset, 16, 16, Qt::AlignCenter, QIcon::Normal);
    }
    
    bool hasSubTitle;
    if (index.data(MediaItem::SubTitleRole).isValid() || 
        index.data(MediaItem::SemanticCommentRole).isValid()) {
        if (!index.data(MediaItem::SubTitleRole).toString().isEmpty() ||
            !index.data(MediaItem::SemanticCommentRole).toString().isEmpty()) {
            hasSubTitle = true;
        } else {
            hasSubTitle = false;
        }
    } else {
        hasSubTitle = false;
    }
    
    //Paint text
    int textInner = m_textInner;
    if (hasCustomArtwork && m_renderMode == NormalMode) {
        textInner = topOffset + iconSize + m_padding;
    }
    QFont textFont;
    if (m_renderMode == NormalMode) {
        QFont textFont = option.font;
    } else {
        textFont = KGlobalSettings::smallestReadableFont();
    }
    int vAlign = (hasSubTitle && m_renderMode == NormalMode) ? Qt::AlignTop : Qt::AlignVCenter;
    int hAlign = (isAction || isMessage) ? Qt::AlignCenter : Qt::AlignLeft;
    int textWidth = (isAction || isMessage) ?
            width - textInner - m_padding : width - textInner - m_padding - m_durRatingSpacer;
    QString text = index.data(Qt::DisplayRole).toString();
    textFont.setItalic(isAction || isMessage);
    p.setFont(textFont);
    p.setPen(foregroundColor);
    p.drawText(left + textInner,
                top+1, textWidth, height,
                vAlign | hAlign, text);
    if (hasSubTitle && m_renderMode == NormalMode) {
        QString subTitle = index.data(MediaItem::SubTitleRole).toString();
        p.setPen(subColor);
        p.drawText(left + textInner,
                    top, textWidth, height,
                    Qt::AlignBottom | hAlign, subTitle);
        QFontMetrics fm(textFont);
        if (fm.width(subTitle) < textWidth) {
            QFont commentFont = KGlobalSettings::smallestReadableFont();
            commentFont.setItalic(true);
            QString spacer  = subTitle.isEmpty() ? QString() : QString("  ");
            QString comment = spacer + index.data(MediaItem::SemanticCommentRole).toString();
            p.setFont(commentFont);
            p.drawText(left + textInner + fm.width(subTitle),
                        top, textWidth - fm.width(subTitle), height,
                        Qt::AlignBottom | hAlign, comment);
            p.setFont(textFont);
        }
    }
    
    //Paint duration
    if (m_renderMode == NormalMode || m_renderMode == MiniMode) {
        QString duration = index.data(MediaItem::DurationRole).toString();
        p.setPen(subColor);
        p.drawText(left + width - m_durRatingSpacer,
                    top+1, m_durRatingSpacer - m_iconSize, height, //icon size = add to / remove from playlist icon
                    Qt::AlignBottom | Qt::AlignRight, duration);
    }
    
    //Paint Rating
    if (m_renderMode == NormalMode || m_renderMode == MiniRatingMode) {
        if ((m_nepomukInited) && 
            (isMediaItem || !index.data(MediaItem::RatingRole).isNull()) && 
            (subType != "CD Track") &&
            (subType != "DVD Title") &&
            hasUrl ) {
            int rating = (index.data(MediaItem::RatingRole).isValid()) ?
                            index.data(MediaItem::RatingRole).toInt() : 0;
            StarRating r = StarRating(rating, m_starRatingSize, ratingRect(&option.rect).topLeft());
            if (option.state.testFlag(QStyle::State_MouseOver))
                r.setHoverAtPosition(m_view->mapFromGlobal(QCursor::pos()));
            r.paint(&p);
        }
    }
    
    //Paint PlayCount
    if (m_renderMode == MiniPlayCountMode && !index.data(MediaItem::PlayCountRole).isNull()) {
        QString playCountText = QString("%1").arg(index.data(MediaItem::PlayCountRole).toInt());
        p.drawText(left + width - m_durRatingSpacer,
                    top+1, m_durRatingSpacer - 1, height,
                    Qt::AlignVCenter| Qt::AlignRight, playCountText);
    }
        
    //Paint Remove from playlist / add to playlist / category icon
    if (isMediaItem && hasUrl && m_renderMode == NormalMode) {
        //Paint add to playlist Icon
        int playlistRow = m_application->playlist()->playlistModel()->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
        QIcon icon;
        if (playlistRow != -1) {
            if (option.state.testFlag(QStyle::State_MouseOver)) { 
                icon = m_removeFromPlaylist;
            } else {
                icon = m_showInPlaylist;
            }
        } else {
            if (option.state.testFlag(QStyle::State_MouseOver)) {
                icon = m_showNotInPlaylist;
            } else {
                icon = KIcon();
            }
        }
        int stateIconWidth = 16;
        int topOffset = (height - m_iconSize) / 2;
        icon.paint(&p, left + width - m_padding - stateIconWidth, top + topOffset, m_iconSize, stateIconWidth, Qt::AlignCenter, QIcon::Normal);
    } else if (isCategory && hasUrl && m_renderMode == NormalMode) {
        //Paint Category Icon
        int stateIconWidth = 22;
        int topOffset = (height - stateIconWidth) / 2;
        m_categoryActionIcon.paint(&p, left + width - stateIconWidth, top + topOffset, stateIconWidth, stateIconWidth, Qt::AlignLeft, QIcon::Normal);
    }
        
    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);
    
}

QSize MediaItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, calcItemHeight());
}

int MediaItemDelegate::calcItemHeight() const
{
    QFont titleFont;
    titleFont.setBold(true);
    
    int textHeight;
    int minHeight;
    int padding;
    if (m_renderMode == NormalMode || m_renderMode == MiniAlbumMode) {
        textHeight = 2 * QFontInfo(titleFont).pixelSize(); // Height for title and subtitle
        minHeight = 22;
        padding = 3;
    } else {
        titleFont = KGlobalSettings::smallestReadableFont();
        textHeight = QFontInfo(titleFont).pixelSize()+4; // Height for title
        minHeight = 0;
        padding = 2;
    }
    return  qMax(textHeight, minHeight) + padding * 2 + 1;
}

int MediaItemDelegate::columnWidth (int column, int viewWidth) const {
    Q_UNUSED(column);
    return viewWidth;
}

bool MediaItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *_model,                                                const QStyleOptionViewItem &option, const QModelIndex &_index)
{
    static bool s_mouseOverRating = false;
    QEvent::Type eventType = event->type();
    QModelIndex index;
    MediaItemModel *model;
    QPoint mousePos = ((QMouseEvent *)event)->pos();
    
    //if we use a proxy the index is from the proxy, not from the MediaItemModel which we need
    if (useProxy()) {
        MediaSortFilterProxyModel * proxy = (MediaSortFilterProxyModel *) _model;
        model = (MediaItemModel *) proxy->sourceModel();
        index = proxy->mapToSource(_index);
    } else {
        model = (MediaItemModel *) _model;
        index = _index;
    }
      
    QString url = index.data(MediaItem::UrlRole).toString();
    QString type = index.data(MediaItem::TypeRole).toString();
    

    if (index.column() != 0) //by default it only has the first column
        goto no_special_event;
    
    /* Upcoming behaviour: 
     * 1. check for the type of the item
     * 2. if exist, check for special regions (rating, "buttons")
     * 3. if they offer different behaviour, check for special events
     */
    if (Utilities::isAction(type)) {
        //no special areas, check event
        if (event->type() == QEvent::MouseButtonPress) {
            emit actionActivated(index);
        }
        return true;
    } else if (Utilities::isMessage(type)) {
        //do nothing
        return true;
    } else if (Utilities::isCategory(type)) {
        if(url.isEmpty()) {
            goto no_special_event;
        }
        QRect curArea = addRmPlaylistRect(&option.rect);
        //doubleclick on the item or click on the "add to/remove from playlist" area
        if ((eventType == QEvent::MouseButtonDblClick) ||
            (eventType == QEvent::MouseButtonPress && curArea.contains(mousePos) && m_renderMode == NormalMode )
            ) {
                emit categoryActivated(index);
                return true;
        }
    } else if (Utilities::isMedia(type)) {
        if (url.isEmpty()) {
            goto no_special_event;
        }
        //check for rating region
        QRect curArea = ratingRect(&option.rect);
        if (curArea.contains(mousePos)) {
            //check for events/if it's useful
            if (!m_nepomukInited ||
                (eventType != QEvent::MouseButtonPress && eventType != QEvent::MouseMove) ||
                (m_renderMode != NormalMode && m_renderMode != MiniRatingMode)
            ) {
                goto no_special_event;
            }
            if (!s_mouseOverRating) {//mouse entered
                s_mouseOverRating = true;
            }
            if (eventType == QEvent::MouseMove) //mouse over
            {
                m_view->update(index);
                return true;
            }
            //else the user clicked, so we have to save the new rating
            int rating = StarRating::RatingAtPosition(mousePos, m_starRatingSize, curArea.topLeft());
            MediaItemModel *cmodel = model;
            //models have to be update
            #define MODELS_TO_BE_UPDATED 5
            MediaItemModel *models[MODELS_TO_BE_UPDATED] = {
                cmodel,
                m_application->playlist()->playlistModel(),
                m_application->playlist()->queueModel(),
                m_application->playlist()->nowPlayingModel(),
                m_application->browsingModel()
            };
            MediaItem updatedItem = cmodel->mediaItemAt(index.row());
            m_mediaIndexer->updateRating(updatedItem.fields["resourceUri"].toString(),rating);
            for (int i = 0; i < MODELS_TO_BE_UPDATED; cmodel = models[++i])
            {
                int row = cmodel->rowOfUrl(url);
                if (row >= 0) {
                    MediaItem update = cmodel->mediaItemAt(row);
                    update.fields["rating"] = rating;
                    cmodel->replaceMediaItemAt(row, update);
                }
            }
            #undef MODELS_TO_BE_UPDATED
            return true;
        } else {
            if (s_mouseOverRating) { //onLeave effect
                s_mouseOverRating = false;
                m_view->update(index);
                return true;
            }
                
        }
        //remove from/add to playlist area
        curArea = addRmPlaylistRect(&option.rect);
        if (curArea.contains(mousePos)) {
            if (eventType == QEvent::MouseButtonPress && m_renderMode == NormalMode ) {
                //clicked on the icon
                int playlistRow = m_application->playlist()->playlistModel()->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
                if (playlistRow != -1) {
                    m_application->playlist()->removeMediaItemAt(playlistRow);
                } else {
                    m_application->playlist()->addMediaItem(model->mediaItemAt(index.row()));
                }  
                return true;
            }
        }
    }
no_special_event:
    return QItemDelegate::editorEvent(event, model, option, index);
}

void MediaItemDelegate::setView(QAbstractItemView * view) 
{
    m_view = view;
    m_defaultViewSelectionMode = view->selectionMode();
}

void MediaItemDelegate::setRenderMode(RenderMode mode)
{
    m_renderMode = mode;
    if (mode == NormalMode || mode == MiniAlbumMode) {
        m_padding = 3;
        m_iconSize = 22;
    } else {
        m_padding = 2;
        m_iconSize = 0;
    }
    m_textInner = m_iconSize == 0 ? m_padding : m_iconSize + 2 * m_padding;

    if (m_renderMode == NormalMode || m_renderMode == MiniRatingMode || m_renderMode == MiniMode) {
        m_durRatingSpacer = 50 + m_iconSize; //m_iconSize for the add to /remove from playlist icon
    } else if (m_renderMode == MiniPlayCountMode) {
        m_durRatingSpacer = 20;
    } else {
        m_durRatingSpacer = 0;
    }

}

QRect MediaItemDelegate::ratingRect(const QRect *rect) const
{
    QSize sz = StarRating::SizeHint(m_starRatingSize);
    QPoint p = QPoint(rect->left() + rect->width() - m_durRatingSpacer,
                      rect->top() + m_padding);
    return QRect(p, sz);
}

QRect MediaItemDelegate::addRmPlaylistRect(const QRect* rect) const
{
    QPoint p = QPoint(rect->left() + rect->width() - m_iconSize,
                      rect->top() + m_padding);
    return QRect(p, QSize(m_iconSize, m_iconSize));
}

MediaItemDelegate::RenderMode MediaItemDelegate::currentRenderMode()
{
    return m_renderMode;
}

int MediaItemDelegate::heightForAllRows()
{
    return m_view->model()->rowCount()*(calcItemHeight())+10;
}

void MediaItemDelegate::setUseProxy(bool b)
{
    m_useProxy = b;
}
