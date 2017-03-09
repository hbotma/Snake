#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <vector>

//////////////////////////////////////////////////
//////////////////////////////////////////////////

enum Direction {UP, LEFT, DOWN, RIGHT};
const int WINDOWX = 360, WINDOWY = 360, BLOCKSIZE = 30;

//////////////////////////////////////////////////
//////////////////////////////////////////////////

struct Spawn
{
    sf::Vector2f point;
    int numBlocks;
};

//////////////////////////////////////////////////
//////////////////////////////////////////////////

class Block : public sf::RectangleShape
{
public:
    Block(float posX, float posY, sf::Color color = sf::Color::Green)
    {
        setFillColor(color);
        setPosition(posX, posY);
        setSize(sf::Vector2f(BLOCKSIZE, BLOCKSIZE));
        setOutlineThickness(-3);
        setOutlineColor(sf::Color::Black);
    }

    Block()
    {
        setFillColor(sf::Color::Green);
        setPosition(BLOCKSIZE, BLOCKSIZE);
        setSize(sf::Vector2f(BLOCKSIZE, BLOCKSIZE));
        setOutlineThickness(-3);
        setOutlineColor(sf::Color::Black);
    }
};

//////////////////////////////////////////////////
//////////////////////////////////////////////////

sf::Vector2f moveCalc(Direction dir)
{
    switch(dir)
    {
        case UP:
            return sf::Vector2f(0, -1 * BLOCKSIZE);
        case LEFT:
            return sf::Vector2f(-1 * BLOCKSIZE, 0);
        case DOWN:
            return sf::Vector2f(0, BLOCKSIZE);
        case RIGHT:
            return sf::Vector2f(BLOCKSIZE, 0);
        default:
            return sf::Vector2f(0,0);
    }
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

Block generateApple(std::vector<Block> & block, int size)
{
    sf::Vector2f pos;
    bool clear = true;

    do
    {
        clear = true;
        pos = sf::Vector2f(BLOCKSIZE * (1 + rand() % ((WINDOWX - 2 * BLOCKSIZE) / BLOCKSIZE)),
                           BLOCKSIZE * (1 + rand() % ((WINDOWY - 2 * BLOCKSIZE) / BLOCKSIZE)));

        for (int i = 0; i < size; i++)
            if (pos == block[i].getPosition())
                clear = false;
    } while (!clear);

    return Block(pos.x, pos.y, sf::Color::Red);
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

bool moveBlocks(std::vector<Block> & block, Block & apple, Spawn & spawn, sf::FloatRect const & field, int & size, Direction dir)
{
    bool success = true;

    for (int i = size-1; i > 0; i--)
        block[i].setPosition(block[i-1].getPosition());
    block[0].move(moveCalc(dir));

    if (!field.contains(block[0].getPosition()))
    {
        success = false;
        block[0].setFillColor(sf::Color::Yellow);
    }
    for (int i = 1; i < size && success; i++)
        if (block[0].getPosition() == block[i].getPosition())
        {
            success = false;
            block[0].setFillColor(sf::Color::Yellow);
        }

    if (block[0].getPosition() == apple.getPosition())
    {
        if (size == 100)
            return false;
        apple = generateApple(block, size);
        spawn.numBlocks += 3;
        spawn.point = block[size-1].getPosition();
    }

    if (spawn.numBlocks > 0)
    {
        block.push_back(Block(spawn.point.x, spawn.point.y));
        spawn.numBlocks--;
        size++;
    }

    return success;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//////////////////////////////////////////////////

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOWX, WINDOWY), "Snake", sf::Style::Close);
    sf::FloatRect field(BLOCKSIZE, BLOCKSIZE, WINDOWX - 2 * BLOCKSIZE, WINDOWY - 2 * BLOCKSIZE);
    sf::Clock clock;
    sf::Time elapsed;

    sf::RectangleShape fieldShape(sf::Vector2f(WINDOWX - 2 * BLOCKSIZE, WINDOWY - 2 * BLOCKSIZE));
    fieldShape.setPosition(BLOCKSIZE, BLOCKSIZE);
    fieldShape.setFillColor(sf::Color::Black);
    fieldShape.setOutlineThickness(3);
    fieldShape.setOutlineColor(sf::Color::Black);
    sf::RectangleShape background(sf::Vector2f(WINDOWX, WINDOWY)), overlay(sf::Vector2f(WINDOWX, WINDOWY));
    overlay.setFillColor(sf::Color(150, 150, 150, 150));
    background.setFillColor(sf::Color(90,50,10));
    sf::Font font;
    sf::Text ggText;

    window.setKeyRepeatEnabled(false);
    srand(time(NULL));

    if (!font.loadFromFile("fonts/comic.ttf"))
    {
        std::cout << "Failed to load resource(s).\n\n";
        return 0;
    }
    ggText.setFont(font);
    ggText.setCharacterSize(40);
    ggText.setPosition(2*BLOCKSIZE, 5*BLOCKSIZE);
    ggText.setColor(sf::Color::Black);
    ggText.setString("Game Over :(");

    std::vector<Block> block;
    Block apple;
    Spawn spawn;
    spawn.numBlocks = 0;

    Direction dir = RIGHT;
    int size = 0;
    bool moved = false, gameOver = false, paused = false;

    block.push_back(Block(BLOCKSIZE, BLOCKSIZE, sf::Color(20,90,10)));
    size++;
    apple = generateApple(block, size);

    clock.restart();

    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (!gameOver)
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    moved = true;

                    if (event.key.code == sf::Keyboard::Up && dir != UP && dir != DOWN)
                        dir = UP;
                    else if (event.key.code == sf::Keyboard::Left && dir != LEFT && dir != RIGHT)
                        dir = LEFT;
                    else if (event.key.code == sf::Keyboard::Down && dir != DOWN && dir != UP)
                        dir = DOWN;
                    else if (event.key.code == sf::Keyboard::Right && dir != RIGHT && dir != LEFT)
                        dir = RIGHT;
                    else
                        moved = false;

                    paused = false;
                }
                else if (event.type == sf::Event::LostFocus)
                    paused = true;
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                gameOver = false;
                size = 0;
                block = std::vector<Block>();
                block.push_back(Block(BLOCKSIZE, BLOCKSIZE, sf::Color(20,90,10)));
                dir = RIGHT;
                size++;
                apple = generateApple(block, size);
                clock.restart();
                elapsed = sf::Time::Zero;
            }
        }

        //////////////////////////////////////////////////
        //////////////////////////////////////////////////

        if (paused)
        {
            clock.restart();
            elapsed = sf::Time::Zero;
            ggText.setString("Paused");
        }
        else
        {
            if (moved)
            {
                clock.restart();
                elapsed = sf::Time::Zero;
            }
            else
            {
                elapsed += clock.restart();
                if (elapsed.asSeconds() >= 0.5)
                {
                    elapsed -= sf::seconds(0.5);
                    moved = true;
                }
            }

            if (moved && !gameOver)
                gameOver = !moveBlocks(block, apple, spawn, field, size, dir);

            moved = false;

            if (size == 100)
                ggText.setString("You Win! :D");
            else
                ggText.setString("Game Over :(");
        }

        //////////////////////////////////////////////////
        //////////////////////////////////////////////////

        window.clear();

        window.draw(background);
        window.draw(fieldShape);
        window.draw(apple);
        for (int i = 1; i < size; i++)
            window.draw(block[i]);
        window.draw(block[0]);
        if (gameOver || paused)
        {
            window.draw(overlay);
            window.draw(ggText);
        }

        window.display();
    }

    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    return 0;
}
