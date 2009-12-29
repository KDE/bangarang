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
#include "platform/utilities.h"
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
#include <QTextOption>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include "mainwindow.h"

NowPlayingDelegate::NowPlayingDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
    m_ratingNotCount = KIcon("rating").pixmap(16, 16, QIcon::Disabled);
    m_ratingCount = KIcon("rating").pixmap(16, 16);
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
        m_mediaIndexer = new MediaIndexer(this);
    } else {
        m_nepomukInited = false; //no resource manager
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
    int padding = 6;
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);    

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
    
    //Create base pixmap
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());
    
    //Paint Artwork
    KIcon icon(index.data(Qt::DecorationRole).value<QIcon>());
    int iconWidth = 128;
    int topOffset = (height - iconWidth) / 2;
    
    if (!icon.isNull()) {
        icon.paint(&p, left + padding, top + topOffset, iconWidth, iconWidth, Qt::AlignCenter, QIcon::Normal);
    } else {
        iconWidth = 0;
    }   
    //Paint text
    int textInner = iconWidth == 0 ? padding : iconWidth + 2 * padding;
    QString text = index.data(Qt::DisplayRole).toString();
    QFont textFont = option.font;
    QTextOption textOption(Qt::AlignLeft | Qt::AlignBottom);
    textOption.setWrapMode(QTextOption::WordWrap);
    QRect titleRect(left + textInner,
                    top + padding, width - textInner - padding, 0.4*iconWidth + topOffset);
    textFont.setPixelSize((int)(titleRect.height()/7));
    p.setFont(textFont);
    p.setPen(foregroundColor);
    p.drawText(QRectF(titleRect), text, textOption);
    
    QString subTitle = index.data(MediaItem::SubTitleRole).toString();
    
    //TODO:Find a place in the UI to show last played and play count
    /*Nepomuk::Resource res(QUrl(index.data(MediaItem::UrlRole).toString()));
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    int playCount = res.property(mediaVocabulary.playCount()).toInt();
    QString lastPlayed = res.property(mediaVocabulary.lastPlayed()).toDateTime().toString();
    subTitle = subTitle + QString(" Last Played %1 Play %2 times")
    .arg(res.property(mediaVocabulary.lastPlayed()).toDateTime().toString())
    .arg(res.property(mediaVocabulary.playCount()).toInt());*/
    
    QFont subTitleFont = option.font;
    QRect subTitleRect(left + textInner,
                     top + topOffset + 0.4*iconWidth, width - textInner - padding, 0.6*iconWidth - 18);
    subTitleFont.setPixelSize((int)(subTitleRect.height()/4));
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QColor subTitleColor = KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    p.setFont(subTitleFont);
    p.setPen(subTitleColor);
    p.drawText(QRectF(subTitleRect), subTitle, textOption);

    //Draw rating
    if (m_nepomukInited && isMediaItem && (subType != "CD Track") && (subType != "DVD Title")) {
        int rating = 0;
        if (index.data(MediaItem::RatingRole).isValid()) {
            rating = int((index.data(MediaItem::RatingRole).toDouble()/2.0) + 0.5);
        }
        for (int i = 1; i <= 5; i++) {
            if (i <= rating) {
                p.drawPixmap(left + textInner + (18 * (i-1)), top + topOffset + iconWidth - 18, m_ratingCount);
            } else {
                p.drawPixmap(left + textInner + (18 * (i-1)), top + topOffset + iconWidth - 18, m_ratingNotCount);
            }
        }
    }
    
    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);
    QPixmap reflection = Utilities::reflection(pixmap);
    painter->drawPixmap(option.rect.topLeft() + QPoint(0, iconWidth+1), reflection);      
    
}

QSize NowPlayingDelegate::sizeHint(const QStyleOptionViewItem &option,                           const QModelIndex &index) const
{
   int width;
   if (index.column() == 0)  {
       width = option.rect.width();
   } else {
       width = 0;
   }
   
   return QSize(width, 300);
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

bool NowPlayingDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,                                                const QStyleOptionViewItem &option, const QModelIndex &index)
{   
    if (event->type() == QEvent::MouseButtonPress) {
        if (index.column() == 0) {
            if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
                //Check if rating was clicked and update rating
                QMouseEvent * mouseEvent = (QMouseEvent *)event;
                int padding = 6;
                int iconWidth = 128;
                int topOffset = (option.rect.height() - iconWidth) / 2;
                int ratingLeft = option.rect.left() + iconWidth + 2 * padding;
                int ratingRight = ratingLeft + 5 * 18;
                int ratingTop = option.rect.top() + topOffset + iconWidth - 18;
                int ratingBottom = option.rect.top() + topOffset + iconWidth;
                if (m_nepomukInited && (mouseEvent->x() > ratingLeft) && (mouseEvent->x() < ratingRight) && (mouseEvent->y() > ratingTop)  && (mouseEvent->y() < ratingBottom)) {
                    int newRating = int ((10.0 * (mouseEvent->x() - ratingLeft)/(5*18)) + 0.5);
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
                    int mediaListRow = m_parent->m_mediaItemModel->rowOfUrl(updatedMediaItem.url);
                    if (mediaListRow != -1) {
                        MediaItem mediaListItem = m_parent->m_mediaItemModel->mediaItemAt(mediaListRow);
                        mediaListItem.fields["rating"] = newRating;
                        m_parent->m_mediaItemModel->replaceMediaItemAt(mediaListRow, mediaListItem);
                    }
                }
            }
        }
    }
    return false;
    Q_UNUSED(model);
    
}
