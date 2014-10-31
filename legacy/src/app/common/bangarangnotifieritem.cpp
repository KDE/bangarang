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

    if (m_currentState == Phonon::StoppedState) {
        setStatus(KStatusNotifierItem::Passive);
        setIconByName("bangarang-notifier");
    } else if (m_currentState == Phonon::PausedState){
        setStatus(KStatusNotifierItem::Active);
        setIconByName("bangarang-notifier-active-pause");
    } else {
        setStatus(KStatusNotifierItem::Active);
        setIconByName("bangarang-notifier-active");
    }
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
