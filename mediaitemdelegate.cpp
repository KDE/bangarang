#include "mediaitemdelegate.h"
#include "platform/playlist.h"
#include "platform/mediaitemmodel.h"
#include "mainwindow.h"

#include <KGlobalSettings>
#include <KColorScheme>
#include <KIcon>
#include <KIconEffect>
#include <Soprano/Vocabulary/NAO>
#include <nepomuk/variant.h>
#include <nepomuk/resource.h>
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
    const int height = calcItemHeight(option);   
    int padding = 3;
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    QColor subColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText) :
    KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    
    
    //Determine item type
    bool isMediaItem = false;
    if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||
        (index.data(MediaItem::TypeRole).toString() == "Video") ||
        (index.data(MediaItem::TypeRole).toString() == "Image")) {
        isMediaItem = true;
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
        //Paint Icon
        KIcon icon(index.data(Qt::DecorationRole).value<QIcon>());
        if (m_parent->m_nowPlaying->rowCount() > 0) {
            MediaItem nowPlayingItem = m_parent->m_nowPlaying->mediaItemAt(0);
            if (nowPlayingItem.url == index.data(MediaItem::UrlRole).toString()) {
                icon = KIcon("media-playback-start");
            }
        }
        int iconWidth = 22;
        int topOffset = (height - iconWidth) / 2;
        if (!icon.isNull()) {
            icon.paint(&p, left + padding, top + topOffset, iconWidth, iconWidth, Qt::AlignCenter, QIcon::Normal);
        } else {
            iconWidth = 0;
        }   
        bool hasSubTitle;
        if (index.data(MediaItem::SubTitleRole).isValid()) {
            if (!index.data(MediaItem::SubTitleRole).toString().isEmpty()) {
                hasSubTitle = true;
            } else {
                hasSubTitle = false;
            }
        } else {
            hasSubTitle = false;
        }
        //Paint text
        int vAlign = hasSubTitle ? Qt::AlignTop : Qt::AlignVCenter;
        int hAlign = (isAction || isMessage) ? Qt::AlignCenter : Qt::AlignLeft;
        int textInner = iconWidth == 0 ? padding : iconWidth + 2 * padding;
        int textWidth = (isAction || isMessage) ? width - textInner- padding : width - textInner - padding - 50;
        QString text = index.data(Qt::DisplayRole).toString();
        QFont textFont = option.font;
        textFont.setItalic(isAction || isMessage);
        p.setFont(textFont);
        p.setPen(foregroundColor);
        p.drawText(left + textInner,
                    top+1, textWidth, height,
                    vAlign | hAlign, text);
        if (hasSubTitle) {
            QString subTitle = index.data(MediaItem::SubTitleRole).toString();
            QFont subTitleFont = KGlobalSettings::smallestReadableFont();
            p.setPen(subColor);
            p.drawText(left + textInner,
                        top, textWidth, height,
                        Qt::AlignBottom | hAlign, subTitle);
        }
        
        //Paint duration
        QString duration = index.data(MediaItem::DurationRole).toString();
        p.setPen(subColor);
        p.drawText(left + width - 50,
                    top+1, 49, height,
                    Qt::AlignBottom | Qt::AlignRight, duration);
        
        //Paint Rating
        if (isMediaItem) {
            int rating = 0;
            if (index.data(MediaItem::RatingRole).isValid()) {
                rating = int((index.data(MediaItem::RatingRole).toDouble()/2.0) + 0.5);
            }
            QPixmap ratingNotCount = KIcon("rating").pixmap(8, 8, QIcon::Disabled);
            QPixmap ratingCount = KIcon("rating").pixmap(8, 8);
            for (int i = 1; i <= 5; i++) {
                if (i <= rating) {
                    p.drawPixmap(left + width - 50 + (10 * (i-1)), top + 3, ratingCount);
                } else {
                    p.drawPixmap(left + width - 50 + (10 * (i-1)), top + 3, ratingNotCount);
                }
            }
        }
        
    } else if (index.column() == 1) {
        //Paint add to playlist Icon
        if (isMediaItem) {
            int playlistRow = m_parent->m_currentPlaylist->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
            QIcon icon;        
            if (playlistRow != -1) {
                icon = KIcon("mail-mark-notjunk");
            } else {
                if (option.state.testFlag(QStyle::State_MouseOver)) {
                    QImage image = KIcon("mail-mark-notjunk").pixmap(16,16).toImage();
                    KIconEffect::toGray(image, 1.0);
                    QPixmap pixmap(16, 16);
                    pixmap.fill(Qt::transparent);
                    QPainter pp(&pixmap);
                    pp.drawImage(QPoint(0,0), image);
                    pp.end();
                    icon = KIcon(pixmap);
                } else {
                    icon = KIcon();
                }
            }
            int iconWidth = 16;
            int topOffset = (height - iconWidth) / 2;
            icon.paint(&p, left + padding , top + topOffset, iconWidth, iconWidth, Qt::AlignCenter, QIcon::Normal);
        }
    } else if (index.column() ==2 ) {
        //Paint Category/Action Icon
        if (isCategory) {
            QIcon catIcon = index.data(Qt::DecorationRole).value<QIcon>();
            int iconWidth = 16;
            int topOffset = (height - iconWidth) / 2;
            catIcon.paint(&p, left + padding , top + topOffset, iconWidth, iconWidth, Qt::AlignCenter, QIcon::Normal);
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
        width = 20;
    } else if (index.column() == 2)  {
        width = 22;
    } else {
        width = 0;
    }
       
    return QSize(width, calcItemHeight(option));
}

int MediaItemDelegate::calcItemHeight(const QStyleOptionViewItem &option) const
{
    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize());
    
    //int textHeight = QFontInfo(titleFont).pixelSize() + QFontInfo(KGlobalSettings::smallestReadableFont()).pixelSize(); // Height for title text and subtitle text
    int textHeight = 2 * QFontInfo(titleFont).pixelSize(); // Height for title text and subtitle text
    return  qMax(textHeight, 22) + 3 * 2 + 1;
}

int MediaItemDelegate::columnWidth (int column, int viewWidth) const {
    if (column == 2) {
        return 16;
    } else {
        return viewWidth - columnWidth(1, viewWidth);
    }
}

bool MediaItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,                                                const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (index.column() == 0) {
            if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
                //Check if rating was clicked and update rating
                QMouseEvent * mouseEvent = (QMouseEvent *)event;
                int ratingLeft = option.rect.right() - 50;
                int ratingRight = option.rect.left();
                int ratingBottom = option.rect.top() + option.rect.height()/2;
                if ((mouseEvent->x() > ratingLeft) && (mouseEvent->y() < ratingBottom)) {
                     int newRating = int ((10.0 * (mouseEvent->x() - ratingLeft)/50.0) + 0.5);
                     MediaItemModel * model = (MediaItemModel *)index.model();
                     MediaItem updatedMediaItem = model->mediaItemAt(index.row());
                     updatedMediaItem.fields["rating"] = newRating;
                     model->replaceMediaItemAt(index.row(), updatedMediaItem);
                     Nepomuk::Resource res(QUrl(updatedMediaItem.url));
                     res.setRating(newRating);
                     //Keep other views of same mediaItem in sync
                     int playlistRow = m_parent->m_playlist->playlistModel()->rowOfUrl(updatedMediaItem.url);
                     if (playlistRow != -1) {
                         MediaItem playlistItem = m_parent->m_playlist->playlistModel()->mediaItemAt(playlistRow);
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
                }
            }
        }
        if (index.column() == 1) {
            if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
                int playlistRow = m_parent->m_currentPlaylist->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
                if (playlistRow != -1) {
                    m_parent->m_playlist->removeMediaItemAt(playlistRow);
                } else {
                    MediaItemModel * model = (MediaItemModel *)index.model();
                    m_parent->m_playlist->addMediaItem(model->mediaItemAt(index.row()));
                }
            }
        }
        if (index.column() == 2) {
            if (index.data(MediaItem::TypeRole).toString() == "Category") {
                m_parent->addListToHistory();
                emit categoryActivated(index);
                return true;
            }
        }
        if (index.data(MediaItem::TypeRole).toString() == "Action") {
            m_parent->addListToHistory();
            emit actionActivated(index);
            return true;
        }
        if (index.data(MediaItem::TypeRole).toString() == "Message") {
            // Do nothing
        }
    }
    
    if (index.column() != 1 && (index.data(MediaItem::TypeRole).toString() != "Message")) {
        return QItemDelegate::editorEvent(event, model, option, index);
    } 
}

void MediaItemDelegate::setView(QTreeView * view) 
{
    m_view = view;
    m_defaultViewSelectionMode = view->selectionMode();
}