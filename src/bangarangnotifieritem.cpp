#include "bangarangnotifieritem.h"

#include <KLocalizedString>
#include <KIcon>
#include <KIconLoader>
#include <QPainter>

BangarangNotifierItem::BangarangNotifierItem(QObject* parent)
  : KStatusNotifierItem(parent)
{
  setTitle(i18n("Bangarang"));
  setIconByName("bangarang");
  
  m_iconSize = KIconLoader::global()->currentSize(KIconLoader::Panel);
  
  m_icon = KIcon("bangarang").pixmap(m_iconSize, m_iconSize);
  m_grayIcon = KIcon("bangarang-notifier-gray").pixmap(m_iconSize, m_iconSize);
  
  setStandardActionsEnabled(false);
  
  m_currentState = Phonon::StoppedState;
  m_muted = false;
  m_pos = -1;
  
  connect(this, SIGNAL(scrollRequested(int,Qt::Orientation)),
    this, SLOT(handleScrollRequested(int,Qt::Orientation)));
  connect(this, SIGNAL(secondaryActivateRequested(QPoint)), this, SLOT(handleMiddleClick()));
}

BangarangNotifierItem::~BangarangNotifierItem()
{ }

Phonon::State BangarangNotifierItem::state() const
{
  return m_currentState;
}

void BangarangNotifierItem::setState(Phonon::State state)
{
  if (m_currentState == state)
    return;
  
  m_currentState = state;
  
  if (m_currentState != Phonon::PlayingState &&
    m_currentState != Phonon::PausedState)
  {
    updateIcon();
    m_pos = 0;
  }
    
  updateOverlayIcon();
}

bool BangarangNotifierItem::isVolumeMuted() const
{
  return m_muted;
}

void BangarangNotifierItem::setVolumeMuted(bool muted)
{
  m_muted = muted;
  
  updateIcon();
}

void BangarangNotifierItem::handleMiddleClick()
{
  if (m_currentState == Phonon::PlayingState)
    emit changeStateRequested(Phonon::PausedState);
  else if (m_currentState == Phonon::PausedState)
    emit changeStateRequested(Phonon::PlayingState);
}

void BangarangNotifierItem::handleScrollRequested(int delta, Qt::Orientation orientation)
{
  if (orientation == Qt::Vertical)
    emit changeVolumeRequested(delta);
  else emit changeTrackRequested(delta);
}

void BangarangNotifierItem::updateAppIcon(qint64 position, qint64 length)
{
  const qint64 pos = (float(position) / length) * m_iconSize;
  
  if (pos == m_pos)
    return;
  
  m_pos = pos;
  
  updateIcon();
}

void BangarangNotifierItem::updateOverlayIcon()
{
  if (m_currentState == Phonon::PlayingState)
    setOverlayIconByName("media-playback-start");
  else if (m_currentState == Phonon::PausedState)
    setOverlayIconByName("media-playback-pause");
  else setOverlayIconByName(QString());
}

void BangarangNotifierItem::updateIcon()
{ 
  QPixmap icon = m_icon;
  
  QPainter p(&icon);
  if (m_pos != 0)
    p.drawPixmap(0, 0, m_grayIcon, 0, 0, 0, m_pos);
  if (m_muted)
  {
    int size = m_iconSize >> 1;
    QPixmap overlay = KIcon("audio-volume-muted").pixmap(size, size);
    const int y = m_iconSize - size;
    
    p.drawPixmap(0, y, overlay);
  }
  p.end();
  
  setIconByPixmap(icon);
}

