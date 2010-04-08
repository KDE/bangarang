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
#include "mainwindow.h"
#include "infomanager.h"
#include "platform/playlist.h"
#include "platform/mediaindexer.h"
#include "platform/mediaitemmodel.h"

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

MediaItemDelegate::MediaItemDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
    m_renderMode = NormalMode;
    m_ratingNotCount = KIcon("rating").pixmap(8, 8, QIcon::Disabled);
    m_ratingCount = KIcon("rating").pixmap(8, 8);
    m_showPlaying = KIcon("media-playback-start");
    m_showInPlaylist = KIcon("mail-mark-notjunk");
    QImage image = KIcon("mail-mark-notjunk").pixmap(16,16).toImage();
    KIconEffect::toGray(image, 1.0);
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter pp(&pixmap);
    pp.drawImage(QPoint(0,0), image);
    pp.end();
    m_showNotInPlaylist = KIcon(pixmap);
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
        m_mediaIndexer = new MediaIndexer(this);
    } else {
        m_nepomukInited = false; //no resource manager
    }
    
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
    
    int padding;
    if (m_renderMode == NormalMode || m_renderMode == MiniAlbumMode) {
        padding = 3;
    } else {
        padding = 2;
    }
    
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    QColor subColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText) :
    KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    QColor nowPlayingColor = option.palette.color(QPalette::Highlight);
    nowPlayingColor.setAlpha(70);
    
    //Determine item type
    bool isMediaItem = false;
    if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||
        (index.data(MediaItem::TypeRole).toString() == "Video") ||
        (index.data(MediaItem::TypeRole).toString() == "Image")) {
        isMediaItem = true;
    }
    QString subType;
    if (index.data(MediaItem::TypeRole).toString() == "Audio") {
        MediaItemModel * model = (MediaItemModel *)index.model();
        subType = model->mediaItemAt(index.row()).fields["audioType"].toString();
    } else if (index.data(MediaItem::TypeRole).toString() == "Video") {
        MediaItemModel * model = (MediaItemModel *)index.model();
        subType = model->mediaItemAt(index.row()).fields["videoType"].toString();
    }
    bool isCategory = index.data(MediaItem::TypeRole).toString() == "Category" ? true : false;
    bool isAction = index.data(MediaItem::TypeRole).toString() == "Action" ? true : false;
    bool isMessage = index.data(MediaItem::TypeRole).toString() == "Message" ? true : false;
    
    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());
    
    if (index.column() == 0) {
        //Paint backgroung for currently playing item
        KIcon icon(index.data(Qt::DecorationRole).value<QIcon>());
        if (m_parent->m_nowPlaying->rowCount() > 0) {
            MediaItem nowPlayingItem = m_parent->m_nowPlaying->mediaItemAt(0);
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
        int iconWidth;
        if (m_renderMode == NormalMode || m_renderMode == MiniAlbumMode) {
            if ((isCategory && index.data(MediaItem::SubTypeRole).toString() == "Album") ||
            (subType == "Movie")) {
                iconWidth = height - 2;
            } else {
                iconWidth = 22;
            }
        } else {
            iconWidth = 0;
        }
        int topOffset = (height - iconWidth) / 2;
        if (topOffset < padding && index.data(MediaItem::SubTypeRole).toString() != "Album" && subType != "Movie") {
            topOffset = padding;
        }
        if (m_renderMode == NormalMode || m_renderMode == MiniAlbumMode) {
            if (!icon.isNull()) {
                icon.paint(&p, left + topOffset, top + topOffset, iconWidth, iconWidth, Qt::AlignCenter, QIcon::Normal);
            }
        }
        if (!exists && m_renderMode == NormalMode) {
            KIcon("emblem-unmounted").paint(&p, left + padding, top + topOffset, 16, 16, Qt::AlignCenter, QIcon::Normal);
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
        QFont textFont;
        if (m_renderMode == NormalMode) {
            QFont textFont = option.font;
        } else {
            textFont = KGlobalSettings::smallestReadableFont();
        }
        int durRatingSpacer;
        if ((m_renderMode == NormalMode && isMediaItem) || m_renderMode == MiniRatingMode || m_renderMode == MiniMode) {
            durRatingSpacer = 50;
        } else if (m_renderMode == MiniPlayCountMode) {
            durRatingSpacer = 20;
        } else {
            durRatingSpacer = 0;
        }
        int vAlign = (hasSubTitle && m_renderMode == NormalMode) ? Qt::AlignTop : Qt::AlignVCenter;
        int hAlign = (isAction || isMessage) ? Qt::AlignCenter : Qt::AlignLeft;
        int textInner = iconWidth == 0 ? padding : iconWidth + 2 * padding;
        int textWidth = (isAction || isMessage) ? width - textInner- padding : width - textInner - padding - durRatingSpacer;
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
            p.drawText(left + width - durRatingSpacer,
                        top+1, durRatingSpacer - 1, height,
                        Qt::AlignBottom | Qt::AlignRight, duration);
        }
        
        //Paint Rating
        if (m_renderMode == NormalMode || m_renderMode == MiniRatingMode) {
            if ((m_nepomukInited) && 
                (isMediaItem || !index.data(MediaItem::RatingRole).isNull()) && 
                (subType != "CD Track") 
                && (subType != "DVD Title")) {
                int rating = 0;
                if (index.data(MediaItem::RatingRole).isValid()) {
                    rating = int((index.data(MediaItem::RatingRole).toDouble()/2.0) + 0.5);
                }
                for (int i = 1; i <= 5; i++) {
                    if (i <= rating) {
                        p.drawPixmap(left + width - durRatingSpacer + (10 * (i-1)), top + 3, m_ratingCount);
                    } else {
                        p.drawPixmap(left + width - durRatingSpacer + (10 * (i-1)), top + 3, m_ratingNotCount);
                    }
                }
            }
        }
        
        //Paint PlayCount
        if (m_renderMode == MiniPlayCountMode && !index.data(MediaItem::PlayCountRole).isNull()) {
            QString playCountText = QString("%1").arg(index.data(MediaItem::PlayCountRole).toInt());
            p.drawText(left + width - durRatingSpacer,
                       top+1, durRatingSpacer - 1, height,
                       Qt::AlignVCenter| Qt::AlignRight, playCountText);
        }
        
    } else if (index.column() == 1) {
        if (isMediaItem) {
            //Paint add to playlist Icon
            int playlistRow = m_parent->m_currentPlaylist->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
            QIcon icon;        
            if (playlistRow != -1) {
                icon = m_showInPlaylist;
            } else {
                if (option.state.testFlag(QStyle::State_MouseOver)) {
                    icon = m_showNotInPlaylist;
                } else {
                    icon = KIcon();
                }
            }
            int iconWidth = 16;
            int topOffset = (height - iconWidth) / 2;
            icon.paint(&p, left + padding , top + topOffset, iconWidth, iconWidth, Qt::AlignCenter, QIcon::Normal);
        } else if (isCategory) {
            //Paint Category Icon
            QIcon catIcon = index.data(Qt::DecorationRole).value<QIcon>();
            int iconWidth = 22;
            int topOffset = (height - iconWidth) / 2;
            catIcon.paint(&p, left, top + topOffset, iconWidth, iconWidth, Qt::AlignLeft, QIcon::Normal);
        }
    }
        
    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);
    
}

QSize MediaItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    int width;
    if (index.column() == 1)  {
        width = 22;
    } else {
        width = 0;
    }
    Q_UNUSED(option);
    return QSize(width, calcItemHeight());
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
    if (column == 1) {
        return 22;
    } else {
        return viewWidth - columnWidth(1, viewWidth);
    }
}

bool MediaItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,                                                const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.column() == 0) {
        if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
            //Check if rating was clicked and update rating
            if (m_nepomukInited && event->type() == QEvent::MouseButtonPress &&
                (m_renderMode == NormalMode || m_renderMode == MiniRatingMode)) {
                QMouseEvent * mouseEvent = (QMouseEvent *)event;
                int ratingLeft = option.rect.right() - 50;
                //int ratingRight = option.rect.left();
                int ratingBottom = option.rect.top() + option.rect.height()/2;
                if ((mouseEvent->x() > ratingLeft) && (mouseEvent->y() < ratingBottom)) {
                     int newRating = int ((10.0 * (mouseEvent->x() - ratingLeft)/50.0) + 0.5);
                     MediaItemModel * model = (MediaItemModel *)index.model();
                     MediaItem updatedMediaItem = model->mediaItemAt(index.row());
                     updatedMediaItem.fields["rating"] = newRating;
                     model->replaceMediaItemAt(index.row(), updatedMediaItem);
                     m_mediaIndexer->updateInfo(updatedMediaItem);
                     //Keep other views of same mediaItem in sync
                     int playlistRow = m_parent->m_playlist->playlistModel()->rowOfUrl(updatedMediaItem.url);
                     if (playlistRow != -1) {
                         MediaItem playlistItem = m_parent->m_playlist->playlistModel()->mediaItemAt(playlistRow);
                         playlistItem.fields["rating"] = newRating;
                         m_parent->m_playlist->playlistModel()->replaceMediaItemAt(playlistRow, playlistItem);
                     }
                     int queueRow = m_parent->m_playlist->queueModel()->rowOfUrl(updatedMediaItem.url);
                     if (queueRow != -1) {
                         MediaItem queueItem = m_parent->m_playlist->queueModel()->mediaItemAt(queueRow);
                         queueItem.fields["rating"] = newRating;
                         m_parent->m_playlist->queueModel()->replaceMediaItemAt(queueRow, queueItem);
                     }
                     int nowPlayingRow = m_parent->m_playlist->nowPlayingModel()->rowOfUrl(updatedMediaItem.url);
                     if (nowPlayingRow != -1) {
                         MediaItem nowPlayingItem = m_parent->m_playlist->nowPlayingModel()->mediaItemAt(nowPlayingRow);
                         nowPlayingItem.fields["rating"] = newRating;
                         m_parent->m_playlist->nowPlayingModel()->replaceMediaItemAt(nowPlayingRow, nowPlayingItem);
                     }
                     int mediaListRow = m_parent->m_mediaItemModel->rowOfUrl(updatedMediaItem.url);
                     if (mediaListRow != -1) {
                         MediaItem mediaListItem = m_parent->m_mediaItemModel->mediaItemAt(mediaListRow);
                         mediaListItem.fields["rating"] = newRating;
                         m_parent->m_mediaItemModel->replaceMediaItemAt(mediaListRow, mediaListItem);
                     }
                }
            }
        } else if (index.data(MediaItem::TypeRole).toString() == "Category") {
            if (event->type() == QEvent::MouseButtonDblClick && (m_renderMode == NormalMode || m_renderMode == MiniAlbumMode)) {
                /*m_parent->addListToHistory();
                MediaItemModel * model = (MediaItemModel *)index.model();
                m_parent->infoManager()->setContext(model->mediaItemAt(index.row()));*/
                emit categoryActivated(index);
            }
        }
    } else if (index.column() == 1) {
        if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
            if (event->type() == QEvent::MouseButtonPress) {
               //Add or remove from playlist
                int playlistRow = m_parent->m_currentPlaylist->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
                if (playlistRow != -1) {
                    m_parent->m_playlist->removeMediaItemAt(playlistRow);
                } else {
                    MediaItemModel * model = (MediaItemModel *)index.model();
                    m_parent->m_playlist->addMediaItem(model->mediaItemAt(index.row()));
                }
            }
        }
        if (index.data(MediaItem::TypeRole).toString() == "Category") {
            if (event->type() == QEvent::MouseButtonPress) {
                /*m_parent->addListToHistory();
                MediaItemModel * model = (MediaItemModel *)index.model();
                m_parent->infoManager()->setContext(model->mediaItemAt(index.row()));*/
                emit categoryActivated(index);
            }
            return true;
        }
    }
    if (index.data(MediaItem::TypeRole).toString() == "Action") {
        if (event->type() == QEvent::MouseButtonPress) {
            //m_parent->addListToHistory();
            emit actionActivated(index);
        }
        return true;
    } else if (index.data(MediaItem::TypeRole).toString() == "Message") {
        // Do nothing
        return true;
    }
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
}

MediaItemDelegate::RenderMode MediaItemDelegate::currentRenderMode()
{
    return m_renderMode;
}

int MediaItemDelegate::heightForAllRows()
{
    return m_view->model()->rowCount()*(calcItemHeight())+10;
}
