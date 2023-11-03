#include "gamescene.h"
#include "utils.h"
#include "pixmapmanager.h"
#include "fontmanager.h"

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QDebug>

QPoint GameScene::s_worldSize = QPoint(20, 10);

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), m_tileSize(40, 20), m_origin(8, 5), m_world(nullptr)
{
    onUserCreate();
}

GameScene::~GameScene()
{

}

void GameScene::loop()
{
    m_deltaTime = m_elapsedTimer.elapsed();
    m_elapsedTimer.restart();

    m_loopTime += m_deltaTime;
    while(m_loopTime > m_loopSpeed)
    {
        m_loopTime -= m_loopSpeed;
        handlePlayerInput();
        handleGui();
        // Work out active cell
        QPoint cell( int(m_mouse->m_x) / m_tileSize.x(), int(m_mouse->m_y) / m_tileSize.y() );

        // Work out mouse offset into cell
        QPoint offset(int(m_mouse->m_x) % m_tileSize.x(), int(m_mouse->m_y) % m_tileSize.y() );

        QImage image(m_sprIsom.toImage().convertToFormat(QImage::Format_ARGB32));

        // Sample into cell offset colour
        QColor col = image.pixelColor(3 * m_tileSize.x() + offset.x(), offset.y());

        // Work out selected cell by transforming screen cell
        QPoint selected(
                           (cell.y() - m_origin.y() + (cell.x() - m_origin.x())),
                           (cell.y() - m_origin.y()) - (cell.x() - m_origin.x())
            );

        // "Bodge" selected cell by sampling corners
        if (col == Qt::red){

            selected += QPoint(-1, +0);
        }
        if (col == Qt::blue)
        {
            selected += QPoint(+0, -1);
        }
        if (col == Qt::green)
        {
            selected += QPoint(+0, +1);
        }
        if (col == Qt::yellow)
        {

            selected += QPoint(+1, +0);
        }

        // Handle mouse click to toggle if a tile is visible or not
        if (m_mouse->m_released)
        {
            // Guard array boundary
            if (selected.x() >= 0 && selected.x() < s_worldSize.x() && selected.y() >= 0 && selected.y() < s_worldSize.y())
            {
                ++m_world[selected.y() * s_worldSize.x() + selected.x()] %= 6;
            }
        }

        // Labmda function to convert "world" coordinate into screen space
        auto ToScreen = [&](int x, int y)
        {
            return QPoint
                {
                          (m_origin.x() * m_tileSize.x()) + (x - y) * (m_tileSize.x() / 2),
                          (m_origin.y() * m_tileSize.y()) + (x + y) * (m_tileSize.y() / 2)
                };
        };


        clear();
        // (0,0) is at top, defined by vOrigin, so draw from top to bottom
        // to ensure tiles closest to camera are drawn last
        for (int y = 0; y < s_worldSize.y(); y++)
        {
            for (int x = 0; x < s_worldSize.x(); x++)
            {
                // Convert cell coordinate to world space
                QPoint vWorld = ToScreen(x, y);
                QGraphicsPixmapItem* pItem = new QGraphicsPixmapItem();
                switch(m_world[y*s_worldSize.x() + x])
                {
                case 0:
                    // Invisble Tile
                    pItem->setPos(vWorld.x(), vWorld.y());
                    pItem->setPixmap(m_sprIsom.copy(1 * m_tileSize.x(), 0, m_tileSize.x(), m_tileSize.y()));
                    break;
                case 1:
                    // Visible Tile
                    pItem->setPos(vWorld.x(), vWorld.y());
                    pItem->setPixmap(m_sprIsom.copy(2 * m_tileSize.x(), 0, m_tileSize.x(), m_tileSize.y()));
                    break;
                case 2:
                    // Tree
                    pItem->setPos(vWorld.x(), vWorld.y() - m_tileSize.y());
                    pItem->setPixmap(m_sprIsom.copy(0 * m_tileSize.x(), 1 * m_tileSize.y(), m_tileSize.x(), m_tileSize.y()*2));
                    break;
                case 3:
                    // Spooky Tree
                    pItem->setPos(vWorld.x(), vWorld.y() - m_tileSize.y());
                    pItem->setPixmap(m_sprIsom.copy(1 * m_tileSize.x(), 1 * m_tileSize.y(), m_tileSize.x(), m_tileSize.y()*2));
                    break;
                case 4:
                    // Beach
                    pItem->setPos(vWorld.x(), vWorld.y() - m_tileSize.y());
                    pItem->setPixmap(m_sprIsom.copy(2 * m_tileSize.x(), 1 * m_tileSize.y(), m_tileSize.x(), m_tileSize.y()*2));
                    break;
                case 5:
                    // Water
                    pItem->setPos(vWorld.x(), vWorld.y() - m_tileSize.y());
                    pItem->setPixmap(m_sprIsom.copy(3 * m_tileSize.x(), 1 * m_tileSize.y(), m_tileSize.x(), m_tileSize.y()*2));
                    break;
                }
                addItem(pItem);
            }
        }

        // Convert selected cell coordinate to world space
        QPoint selectedWorld = ToScreen(selected.x(), selected.y());
        QGraphicsPixmapItem *highlightItem = new QGraphicsPixmapItem();
        highlightItem->setPos(selectedWorld.x(), selectedWorld.y());
        highlightItem->setPixmap(m_sprIsom.copy(0,0, m_tileSize.x(), m_tileSize.y()));
        addItem(highlightItem);

        QGraphicsSimpleTextItem* mouseTextItem = new QGraphicsSimpleTextItem();
        mouseTextItem->setFont(m_font);
        mouseTextItem->setPos(4,4);
        mouseTextItem->setText("Mouse : " + QString::number(m_mouse->m_x) + ", " + QString::number(m_mouse->m_y));
        addItem(mouseTextItem);

        QGraphicsSimpleTextItem* cellTextItem = new QGraphicsSimpleTextItem();
        cellTextItem->setFont(m_font);
        cellTextItem->setPos(4,14);
        cellTextItem->setText("Cell : " + QString::number(cell.x()) + ", " + QString::number(cell.y()));
        addItem(cellTextItem);

        QGraphicsSimpleTextItem* selectedTextItem = new QGraphicsSimpleTextItem();
        selectedTextItem->setFont(m_font);
        selectedTextItem->setPos(4,24);
        selectedTextItem->setText("Selected : " + QString::number(selected.x()) + ", " + QString::number(selected.y()));
        addItem(selectedTextItem);


        drawGui();
        resetStatus();
    }
}

void GameScene::onUserCreate()
{
    setBackgroundBrush(COLOR_STYLE::BACKGROUND);
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i] = new KeyStatus();
    }
    m_mouse = new MouseStatus();
    setSceneRect(0,0, SCREEN::PHYSICAL_SIZE.width(), SCREEN::PHYSICAL_SIZE.height());
    connect(&m_timer, &QTimer::timeout, this, &GameScene::loop);
    m_timer.start(int(1000.0f/FPS));
    m_elapsedTimer.start();

    // Load sprites used in demonstration
    m_sprIsom = PixmapManager::Instance()->getPixmap(PixmapManager::TextureID::All);

    // Create empty world
    m_world = new int[GameScene::s_worldSize.x() * GameScene::s_worldSize.y()]{ 0 };

    m_font = FontManager::Instance()->getFont(FontManager::FontID::GUI);
}

void GameScene::renderScene()
{
    static int index = 0;
    QString fileName = QDir::currentPath() + QDir::separator() + "screen" + QString::number(index++) + ".png";
    QRect rect = sceneRect().toAlignedRect();
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    render(&painter);
    image.save(fileName);
    qDebug() << "saved " << fileName;
}

void GameScene::handlePlayerInput()
{
    if(m_keys[KEYBOARD::KeysMapper[Qt::Key_Z]]->m_released)
    {
        //renderScene();//uncoment if want to make screenshots
    }
}

void GameScene::handleGui()
{
    if(m_mouse->m_released)
    {
        //screenshot
        if(m_mouse->m_x > 0 && m_mouse->m_x < 150 && m_mouse->m_y > sceneRect().height() - 30 && m_mouse->m_y < sceneRect().height())
        {
            renderScene();
        }
        //left
        else if(m_mouse->m_x > 160 && m_mouse->m_x < 180 && m_mouse->m_y > sceneRect().height() - 30 && m_mouse->m_y < sceneRect().height())
        {
            //1-20
            s_worldSize.setX(s_worldSize.x()+1);
            if(s_worldSize.x()>20){
                s_worldSize.setX(20);
            }
        }
        //right
        else if(m_mouse->m_x > 250 && m_mouse->m_x < 280 && m_mouse->m_y > sceneRect().height() - 30 && m_mouse->m_y < sceneRect().height())
        {
            //1-20
            s_worldSize.setX(s_worldSize.x()-1);
            if(s_worldSize.x()<1){
                s_worldSize.setX(1);
            }
        }
        //up
        else if(m_mouse->m_x > 290 && m_mouse->m_x < 320 && m_mouse->m_y > sceneRect().height() - 30 && m_mouse->m_y < sceneRect().height())
        {
            //1-10
            s_worldSize.setY(s_worldSize.y()+1);
            if(s_worldSize.y()>10){
                s_worldSize.setY(10);
            }
        }
        //down
        else if(m_mouse->m_x > 380 && m_mouse->m_x < 410 && m_mouse->m_y > sceneRect().height() - 30 && m_mouse->m_y < sceneRect().height())
        {
            //1-10
            s_worldSize.setY(s_worldSize.y()-1);
            if(s_worldSize.y()<1){
                s_worldSize.setY(1);
            }
        }
        //clear
        else if(m_mouse->m_x > sceneRect().width()-90 && m_mouse->m_x < sceneRect().width() && m_mouse->m_y > sceneRect().height() - 30 && m_mouse->m_y < sceneRect().height())
        {
            for(int i = 0 ; i < GameScene::s_worldSize.x() * GameScene::s_worldSize.y(); ++i)
            {
                m_world[i] = 0;
            }
        }
    }
}

void GameScene::drawGui()
{
    QGraphicsRectItem* rItem = new QGraphicsRectItem(0,0,150,30);
    rItem->setPos(0, sceneRect().height() - 30);
    rItem->setPen(QPen(QColor(200, 10, 10)));
    rItem->setBrush(QBrush(QColor(200, 10, 10)));
    addItem(rItem);

    QFont font = m_font;
    font.setPixelSize(30);
    QGraphicsSimpleTextItem* tItem = new QGraphicsSimpleTextItem(rItem);
    tItem->setFont(font);
    tItem->setText("Screenshot");

    // left/right
    QGraphicsRectItem* p0Item = new QGraphicsRectItem(0,0,30,30);
    p0Item->setPos(160, sceneRect().height() - 30);
    p0Item->setPen(QPen(QColor(10, 200, 10)));
    p0Item->setBrush(QBrush(QColor(10, 200, 10)));
    addItem(p0Item);
    QGraphicsSimpleTextItem* tp0Item = new QGraphicsSimpleTextItem(p0Item);
    tp0Item->setFont(font);
    tp0Item->setText(" +");

    QGraphicsRectItem* middle0Item = new QGraphicsRectItem(0,0,60,30);
    middle0Item->setPos(190, sceneRect().height() - 30);
    middle0Item->setPen(QPen(QColor(10, 10, 200)));
    middle0Item->setBrush(QBrush(QColor(10, 10, 200)));
    addItem(middle0Item);
    QGraphicsSimpleTextItem* m0tItem = new QGraphicsSimpleTextItem(middle0Item);
    m0tItem->setFont(font);
    m0tItem->setText(" <=>");

    QGraphicsRectItem* m0Item = new QGraphicsRectItem(0,0,30,30);
    m0Item->setPos(250, sceneRect().height() - 30);
    m0Item->setPen(QPen(QColor(10, 200, 10)));
    m0Item->setBrush(QBrush(QColor(10, 200, 10)));
    addItem(m0Item);
    QGraphicsSimpleTextItem* tm0Item = new QGraphicsSimpleTextItem(m0Item);
    tm0Item->setFont(font);
    tm0Item->setText(" -");

    // up/down
    QGraphicsRectItem* p1Item = new QGraphicsRectItem(0,0,30,30);
    p1Item->setPos(290, sceneRect().height() - 30);
    p1Item->setPen(QPen(QColor(10, 200, 10)));
    p1Item->setBrush(QBrush(QColor(10, 200, 10)));
    addItem(p1Item);
    QGraphicsSimpleTextItem* tp1Item = new QGraphicsSimpleTextItem(p1Item);
    tp1Item->setFont(font);
    tp1Item->setText(" +");

    QGraphicsRectItem* middle1Item = new QGraphicsRectItem(0,0,60,30);
    middle1Item->setPos(320, sceneRect().height() - 30);
    middle1Item->setPen(QPen(QColor(10, 10, 200)));
    middle1Item->setBrush(QBrush(QColor(10, 10, 200)));
    addItem(middle1Item);
    QGraphicsSimpleTextItem* m1tItem = new QGraphicsSimpleTextItem(middle1Item);
    m1tItem->setFont(font);
    m1tItem->setText("  ^v");

    QGraphicsRectItem* m1Item = new QGraphicsRectItem(0,0,30,30);
    m1Item->setPos(380, sceneRect().height() - 30);
    m1Item->setPen(QPen(QColor(10, 200, 10)));
    m1Item->setBrush(QBrush(QColor(10, 200, 10)));
    addItem(m1Item);
    QGraphicsSimpleTextItem* tm1Item = new QGraphicsSimpleTextItem(m1Item);
    tm1Item->setFont(font);
    tm1Item->setText(" -");

    //clean
    QGraphicsRectItem* cItem = new QGraphicsRectItem(0,0,90,30);
    cItem->setPos(sceneRect().width()-90, sceneRect().height()-30);
    cItem->setPen(QPen(QColor(200, 10, 10)));
    cItem->setBrush(QBrush(QColor(200, 10, 10)));
    addItem(cItem);
    QGraphicsSimpleTextItem* ctItem = new QGraphicsSimpleTextItem(cItem);
    ctItem->setFont(font);
    ctItem->setText("  Clear");
}

void GameScene::resetStatus()
{
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i]->m_released = false;
    }
    for(int i = 0; i < 256; ++i)
    {
        m_keys[i]->m_pressed = false;
    }
    m_mouse->m_released = false;
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        if(event->isAutoRepeat())
        {
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = true;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_pressed = false;
        }
        else
        {
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_pressed = true;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held    = false;
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if(KEYBOARD::KeysMapper.contains(event->key()))
    {
        if(!event->isAutoRepeat())
        {
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_held = false;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_pressed = false;
            m_keys[KEYBOARD::KeysMapper[event->key()]]->m_released = true;
        }

    }
    QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    m_mouse->m_pressed = true;
    QGraphicsScene::mousePressEvent(event);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_mouse->m_x = event->scenePos().x();
    m_mouse->m_y = event->scenePos().y();
    m_mouse->m_pressed = false;
    m_mouse->m_released = true;
    QGraphicsScene::mouseReleaseEvent(event);
}
