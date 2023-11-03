#include "pixmapmanager.h"

PixmapManager* PixmapManager::ptr = nullptr;

PixmapManager *PixmapManager::Instance()
{
    if(!ptr)
    {
        ptr = new PixmapManager();
    }
    return ptr;
}

QPixmap& PixmapManager::getPixmap(TextureID id)
{
    return m_textures.get(id);
}

PixmapManager::PixmapManager()
{
    m_textures.load(TextureID::All, ":/res/isometric_demo.png");
    //std::unique_ptr<QPixmap> ButtonSelected(new QPixmap(getPixmap(TextureID::Buttons).copy(0,50,200,50)));
    //m_textures.insertResource(TextureID::ButtonSelected, std::move(ButtonSelected));
}
