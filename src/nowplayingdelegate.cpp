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
#include "starrating.h"
#include <ui_mainwindow.h>

NowPlayingDelegate::NowPlayingDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
    m_globalRatingRect = QRect(0, 0, 0, 0);
    m_view = NULL;
    m_iconSize = 128;
    m_padding = 6;
    m_textInner = m_iconSize + 2 * m_padding;
    m_starRatingSize = StarRating::Big;

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

    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected)) ?
        option.palette.color(QPalette::HighlightedText) :
        option.palette.color(QPalette::Text);

    bool isMedia = isMediaItem(&index);
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
    int topOffset = (height - m_iconSize) / 2;
    
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
    QRect subTitleRect(left + m_textInner,
                       top + topOffset + 0.4 * m_iconSize,
                       width - m_textInner - m_padding,
                       0.6 * m_iconSize - 18);
    subTitleFont.setPixelSize((int)(subTitleRect.height()/4));
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
            r.setHoverAtPosition(m_view->mapFromGlobal(QCursor::pos()) - option.rect.topLeft());
        r.paint(&p);
    }

    p.end();

    //Draw finished pixmap
    painter->drawPixmap(option.rect.topLeft(), pixmap);
    QPixmap reflection = Utilities::reflection(pixmap);
    painter->drawPixmap(option.rect.topLeft() + QPoint(0, m_iconSize + 1), reflection);
    
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

bool NowPlayingDelegate::isMediaItem(const QModelIndex *index) const
{
    QString type = index->data(MediaItem::TypeRole).toString();
    return
        (
            (type == "Audio") ||
            (type == "Video") ||
            (type == "Video")
        );
}

QRect NowPlayingDelegate::ratingRect(const QRect *rect) const
{
    QSize sz = StarRating::SizeHint(m_starRatingSize);
    QPoint p = QPoint(rect->left() + m_textInner,
                      rect->top() + (rect->height() + m_iconSize) / 2 - sz.height());
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
  if (!isMediaItem(&index))
      return false;

  QPoint mousePos = ((QMouseEvent *)event)->pos();
  QRect ratingArea = ratingRect(&option.rect);
  if (!m_nepomukInited)
      return false;
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
      m_parent->m_playlist->playlistModel(),
      m_parent->m_playlist->queueModel(),
      m_parent->m_mediaItemModel
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
          m_mediaIndexer->updateInfo(update);
          indexerUpdated = true;
      }
  }
  return false;
}