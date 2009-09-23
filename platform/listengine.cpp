#include "listengine.h"

ListEngine::ListEngine(ListEngineFactory * parent) : QThread(parent)
{
}

ListEngine::~ListEngine()
{
}

void ListEngine::setModel(MediaItemModel * mediaItemModel)
{
    m_mediaItemModel = mediaItemModel;
}

MediaItemModel * ListEngine::model()
{
    return m_mediaItemModel;
}