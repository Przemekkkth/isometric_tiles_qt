#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QTimer>
#include <QImage>
#include <QGraphicsSimpleTextItem>
#include <QPoint>
#include <QPixmap>
#include <QFont>

struct KeyStatus
{
    bool m_pressed = false;
    bool m_held = false;
    bool m_released = false;
};

struct MouseStatus
{
    float m_x = 0.0f;
    float m_y = 0.0f;
    bool m_released = false;
    bool m_pressed = false;
};

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();
private slots:
    void loop();

private:
    void onUserCreate();
    void renderScene();
    void handlePlayerInput();
    void handleGui();
    void drawGui();
    void resetStatus();
    KeyStatus* m_keys[256];
    MouseStatus* m_mouse;
    const int FPS = 60;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    float m_deltaTime = 0.0f, m_loopTime = 0.0f;
    const float m_loopSpeed = int(1000.0f/FPS);

    // Number of tiles in world
    static QPoint s_worldSize;
    // Size of single tile graphic
    QPoint m_tileSize;
    // Where to place tile (0,0) on screen (in tile size steps)
    QPoint m_origin;
    // Pointer to create 2D world array
    int *m_world;
    // Sprite that holds all imagery
    QPixmap m_sprIsom;
    // App font
    QFont m_font;
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};


#endif // GAMESCENE_H
