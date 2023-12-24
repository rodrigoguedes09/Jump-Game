#include "gamescene.h"
#include "game.h"
#include <QKeyEvent>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QDir>
#include <QPainter>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene{parent}, m_iteration_value(1000.0f/60.0f),
    m_leftMove(false), m_rightMove(false), m_juliaXpos(100), m_juliaYpos(100),
    m_deltaX(3), m_deltaY(0.2f), m_height(200), m_facingRight(true), m_countOfPlatforms(10)
{
    Game::init();
    srand(time(0));
    setSceneRect(0, 0, m_game.RESOLUTION.width(), m_game.RESOLUTION.height());
    //
    setBackgroundBrush(Qt::white);
    m_juliaPixmap.load(m_game.PATH_TO_JULIA_PIXMAP);
    m_juliaItem = new QGraphicsPixmapItem(QPixmap(m_juliaPixmap));
    m_juliaTransform = m_juliaItem->transform();
    addItem(m_juliaItem);
    //
    m_numberPixmap.load(m_game.PATH_TO_ALL_NUMBERS_PIXMAP);
    //
    m_platformPixmap.load(m_game.PATH_TO_PLATFORM_PIXMAP);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameScene::update);
    m_timer->start(m_iteration_value);

    for (int i= 0; i < m_countOfPlatforms;i++)
    {
        m_platforms[i].x = rand() % Game::RESOLUTION.width();
        m_platforms[i].y = rand() % Game::RESOLUTION.height();
    }
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    {
        if( m_game.STATE == Game::State::Active )
        {
            m_leftMove = true;
        }
    }

    break;
    case Qt::Key_Right:
    {
        if( m_game.STATE == Game::State::Active)
        {
            m_rightMove = true;
        }
    }

    break;
    case Qt::Key_P:
    {
        if(m_game.STATE == Game::State::Active)
        {
            m_game.STATE = Game::State::Paused;
        }
        else if(m_game.STATE == Game::State::Paused)
        {
            m_game.STATE = Game::State::Active;
        }
    }
    break;
    case Qt::Key_R:
    {
        if(m_game.STATE == Game::State::Game_Over)
        {
            reset();
        }
    }
    break;
    case Qt::Key_Z:
    {
        renderScene();//uncomment if you want to make screenshots
    }
    break;

    }

    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
        m_rightMove = false;
        m_leftMove = false;
        break;

    }
    QGraphicsScene::keyReleaseEvent(event);
}

void GameScene::clampXpos() //movimento da julia
{
    if(m_juliaTransform.m11() == -1 ) //isTurnLeft
    {
        if(m_juliaXpos - m_juliaItem->boundingRect().width() < 0)
        {
            m_juliaXpos = m_juliaItem->boundingRect().width();
        }
    }
    else //isTurnRight
    {
        if(m_juliaXpos + m_juliaItem->boundingRect().width() > Game::RESOLUTION.width())
        {
            m_juliaXpos = Game::RESOLUTION.width() - m_juliaItem->boundingRect().width();
        }
    }
}

void GameScene::drawScore()
{
    QString scoreText = QString::number(m_game.POINTS);
    int unityPartVal = 0;
    int decimalPartValue = 0;
    int hendredthPartValue = 0;

    if(scoreText.length() == 1) // 0 - 9
    {
        unityPartVal = scoreText.toInt();
        decimalPartValue = 0;
        hendredthPartValue = 0;
    }
    else if(scoreText.length() == 2) // 10 - 99
    {
        unityPartVal = scoreText.last(1).toInt();
        decimalPartValue = scoreText.first(1).toInt();
        hendredthPartValue = 0;
    }
    else if(scoreText.length() == 3) // 100 - 999
    {
        unityPartVal = scoreText.last(1).toInt();
        hendredthPartValue = scoreText.first(1).toInt();
        QString copyVal = scoreText;
        copyVal.chop(1);
        decimalPartValue = copyVal.last(1).toInt();
    }

    QGraphicsPixmapItem* unityPartScoreItem = new QGraphicsPixmapItem(m_numberPixmap.copy(unityPartVal * Game::NUMBER_SIZE.width(), 0, Game::NUMBER_SIZE.width(), Game::NUMBER_SIZE.width()));
    unityPartScoreItem->moveBy( Game::RESOLUTION.width() - Game::NUMBER_SIZE.width(), 0);
    addItem(unityPartScoreItem);

    QGraphicsPixmapItem* decimalPartScoreItem = new QGraphicsPixmapItem(m_numberPixmap.copy(decimalPartValue * Game::NUMBER_SIZE.width(), 0, Game::NUMBER_SIZE.width(), Game::NUMBER_SIZE.width()));
    decimalPartScoreItem->moveBy(Game::RESOLUTION.width() - 2 * Game::NUMBER_SIZE.width(), 0);
    addItem(decimalPartScoreItem);

    QGraphicsPixmapItem* hundrethPartScoreItem = new QGraphicsPixmapItem(m_numberPixmap.copy(hendredthPartValue * Game::NUMBER_SIZE.width(), 0, Game::NUMBER_SIZE.width(), Game::NUMBER_SIZE.height()));
    hundrethPartScoreItem->moveBy(Game::RESOLUTION.width() - 3 * Game::NUMBER_SIZE.width(), 0);
    addItem(hundrethPartScoreItem);
}

void GameScene::reset()
{
    m_juliaXpos = 100;
    m_juliaYpos = 100;
    m_height = 200;
    for (int i= 0; i < m_countOfPlatforms;i++)
    {
        m_platforms[i].x = rand() % Game::RESOLUTION.width();
        m_platforms[i].y = rand() % Game::RESOLUTION.height();
    }

    m_game.STATE = Game::State::Active;
    m_game.POINTS = 0;
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

void GameScene::update()
{
    clear();
    m_juliaItem = new QGraphicsPixmapItem(QPixmap(m_juliaPixmap.copy(0, 0, Game::JULIA_SIZE.width(), Game::JULIA_SIZE.height())));
    QGraphicsPixmapItem *bgIteam = new QGraphicsPixmapItem(QPixmap(m_game.PATH_TO_BACKGROUND_PIXMAP).scaled(Game::RESOLUTION.width(), Game::RESOLUTION.height()));

    addItem(bgIteam);
    m_time_since_last_iteration += m_iteration_value;
    while(m_time_since_last_iteration > Game::DELAY && m_game.STATE == Game::State::Active)
    {
        m_time_since_last_iteration = -Game::DELAY;
        if(m_leftMove)
        {
            m_juliaXpos -= m_deltaX;
            if(m_juliaTransform.m11() != -1)
            {
                m_juliaTransform = m_juliaTransform.scale(-1, 1);
                m_facingRight = false;
                m_juliaXpos += m_juliaItem->boundingRect().width();
            }

        }
        else if(m_rightMove)
        {
            m_juliaXpos += m_deltaX;
            if(m_juliaTransform.m11() != 1)
            {
                m_juliaTransform = m_juliaItem->transform();
                m_facingRight = true;
                m_juliaXpos -= m_juliaItem->boundingRect().width();
            }
        }


        m_deltaY += Game::JUMP_SPEED;
        m_juliaYpos += m_deltaY;
        if ( m_juliaYpos > Game::DEAD_LEVEL)
        {
            m_deltaY = Game::JUMP_FORCE;
            m_game.STATE = Game::State::Game_Over;
        }

        if (m_juliaYpos < m_height)
        {
            for (int i= 0; i < m_countOfPlatforms; i++)
            {
                m_juliaYpos = m_height;
                m_platforms[i].y = m_platforms[i].y - m_deltaY;
                if (m_platforms[i].y > Game::RESOLUTION.height())
                {
                    m_game.POINTS++;
                    m_platforms[i].y = 0;
                    m_platforms[i].x = rand() % Game::RESOLUTION.width();
                }
            }
        }

        clampXpos();
        for (int i = 0 ; i < m_countOfPlatforms; i++)
        {
            if(m_facingRight)
            {
                if ( (m_juliaXpos + Game::X_OFFSET > m_platforms[i].x) && (m_juliaXpos + Game::X_OFFSET < (m_platforms[i].x + Game::PLATFORM_SIZE.width()))
                    && (m_juliaYpos + Game::Y_OFFSET > m_platforms[i].y) && (m_juliaYpos + Game::Y_OFFSET < (m_platforms[i].y + Game::PLATFORM_SIZE.height())) && (m_deltaY > 0))
                {
                    m_deltaY = Game::JUMP_FORCE;
                }
            }
            else
            {
                if ( (m_juliaXpos - Game::X_OFFSET > m_platforms[i].x) && (m_juliaXpos - Game::X_OFFSET < (m_platforms[i].x + Game::PLATFORM_SIZE.width()))
                    && (m_juliaYpos + Game::Y_OFFSET > m_platforms[i].y) && (m_juliaYpos + Game::Y_OFFSET < (m_platforms[i].y + Game::PLATFORM_SIZE.height())) && (m_deltaY > 0))
                {
                    m_deltaY = Game::JUMP_FORCE;
                }
            }
        }
    }

    clampXpos();
    for(int i = 0; i < m_countOfPlatforms; ++i)
    {
        QGraphicsPixmapItem* platform_item = new QGraphicsPixmapItem(m_platformPixmap.scaled(Game::PLATFORM_SIZE.width(), Game::PLATFORM_SIZE.height()));
        platform_item->setPos(m_platforms[i].x, m_platforms[i].y);
        addItem(platform_item);
    }

    if( m_deltaY < 0)
    {
        //jump pixmap
        m_juliaItem->setPixmap(m_juliaPixmap.copy(Game::JULIA_SIZE.width(), 0, Game::JULIA_SIZE.width(), Game::JULIA_SIZE.height()));
    }
    else
    {
        //fall pixmap
        m_juliaItem->setPixmap(m_juliaPixmap.copy(0, 0, Game::JULIA_SIZE.width(), Game::JULIA_SIZE.height()));
    }
    m_juliaItem->setTransform(m_juliaTransform);
    m_juliaItem->setPos(m_juliaXpos, m_juliaYpos);
    addItem(m_juliaItem);
    drawScore();

    if(m_game.STATE == Game::State::Paused)
    {
        QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(QPixmap(m_game.PATH_TO_PAUSED_BG).scaled(Game::RESOLUTION.width(), Game::RESOLUTION.height()));
        addItem(bgItem);
    }
    else if(m_game.STATE == Game::State::Game_Over)
    {
        QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(QPixmap(m_game.PATH_TO_GAME_OVER_BG).scaled(Game::RESOLUTION.width(), Game::RESOLUTION.height()));
        addItem(bgItem);
    }

    QGraphicsRectItem* rItem = new QGraphicsRectItem();
    rItem->setBrush(backgroundBrush().color());
    rItem->setPen(backgroundBrush().color());
    rItem->setRect(0,0, 100, m_game.RESOLUTION.height());
    rItem->setPos(m_game.RESOLUTION.width(), 0);
    addItem(rItem);

    QGraphicsRectItem* bItem = new QGraphicsRectItem();
    bItem->setBrush(backgroundBrush().color());
    bItem->setPen(backgroundBrush().color());
    bItem->setRect(0,0, m_game.RESOLUTION.width()+100, 100);
    bItem->setPos(0, m_game.RESOLUTION.height());
    addItem(bItem);
}
