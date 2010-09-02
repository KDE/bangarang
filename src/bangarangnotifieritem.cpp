#include "bangarangnotifieritem.h"

#include <KLocalizedString>
#include <KIcon>
#include <KIconLoader>
#include <QPainter>

BangarangNotifierItem::BangarangNotifierItem(QObject* parent)
  : KStatusNotifierItem(parent)
{
  setTitle(i18n("Bangarang"));
  setIconByName("bangarang-notifier");
  
  setStandardActionsEnabled(false);
  
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
  
  if (m_currentState == Phonon::StoppedState)
  {
    setToolTip("bangarang-notifier", i18n("Not Playing"), QString());
    setStatus(KStatusNotifierItem::Passive);
  }
  else setStatus(KStatusNotifierItem::Active);
 
  updateOverlayIcon();
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
    emit changeVolumeRequested(delta/120);
 // else emit changeTrackRequested(delta/120);
}

void BangarangNotifierItem::updateOverlayIcon()
{
  if (m_currentState == Phonon::PlayingState)
    setOverlayIconByName("media-playback-start");
  else if (m_currentState == Phonon::PausedState)
    setOverlayIconByName("media-playback-pause");
  else setOverlayIconByName(QString());
}
