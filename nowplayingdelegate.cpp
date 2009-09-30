#include "nowplayingdelegate.h"
#include "platform/utilities.h"
#include "platform/mediaitemmodel.h"
#include "platform/mediavocabulary.h"

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
#include "mainwindow.h"

NowPlayingDelegate::NowPlayingDelegate(QObject *parent) : QItemDelegate(parent)
{
    m_parent = (MainWindow *)parent;
}

NowPlayingDelegate::~NowPlayingDelegate()
{
}

void NowPlayingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() != 0 && index.column() !=0) {
        return;
    }
    
    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);    
    
    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = option.rect.height();   
    int padding = 6;
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
    
    
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
        icon.paint(&p, left + padding, top + topOffset, iconWidth, iconWidth, Qt::AlignLeft, QIcon::Normal);
    } else {
        iconWidth = 0;
    }   
    //Paint text
    int textInner = iconWidth == 0 ? padding : iconWidth + 2 * padding;
    QString text = index.data(Qt::DisplayRole).toString();
    QFont textFont = option.font;
    QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::WordWrap);
    QRect titleRect(left + textInner,
                     top + topOffset, width - textInner - padding, iconWidth/2);
    textFont.setPixelSize((int)(titleRect.height()/3.5));
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
                     top + topOffset + iconWidth/2, width - textInner - padding, 40+iconWidth/2);
    subTitleFont.setPixelSize((int)(subTitleRect.height()/7));
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QColor subTitleColor = KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    p.setFont(subTitleFont);
    p.setPen(subTitleColor);
    p.drawText(QRectF(subTitleRect), subTitle, textOption);

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
    return QItemDelegate::editorEvent(event, model, option, index);
}
