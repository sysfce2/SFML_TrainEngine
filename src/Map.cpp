#include "Map.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include "FileManager.h"
#include "Tile.h"

Map::Map(const ResourceManager& resourceManager)
    : m_resourceManager(resourceManager),
      m_indexDimensions(0, 0),
      m_maxDimensions(4096, 4096),
      m_layerCount(static_cast<unsigned int>(MapLayer::Count)),
      m_tileSize(64),
      m_displayGrid(false)
{
    m_tiles.resize(m_layerCount);

    m_horizGridLine.setFillColor(sf::Color(0, 0, 0, 128));
    m_vertGridLine.setFillColor(sf::Color(0, 0, 0, 128));

    for (auto& rLayerColor : m_layerColors)
    {
        rLayerColor = sf::Color::White;
    }
}

Map::~Map()
{
    Clear();
}

void Map::draw(sf::RenderTarget& rTarget, sf::RenderStates states) const
{
    sf::Vector2f viewPosition = rTarget.getView().getCenter();
    sf::Vector2f viewDimensions = rTarget.getView().getSize();

    float viewLeft = (viewPosition.x - viewDimensions.x / 2) / m_tileSize;
    if (viewLeft < 0)
    {
        viewLeft = 0;
    }

    float viewRight = (viewPosition.x + viewDimensions.x / 2) / m_tileSize;
    if (viewRight > m_indexDimensions.x)
    {
        viewRight = m_indexDimensions.x;
    }

    float viewTop = (viewPosition.y - viewDimensions.y / 2) / m_tileSize;
    if (viewTop < 0)
    {
        viewTop = 0;
    }

    float viewBottom = (viewPosition.y + viewDimensions.y / 2) / m_tileSize;
    if (viewBottom > m_indexDimensions.y)
    {
        viewBottom = m_indexDimensions.y;
    }

    for (unsigned int z = 0; z < m_layerCount; z++)
    {
        for (unsigned int y = viewTop; y < viewBottom; y++)
        {
            for (unsigned int x = viewLeft; x < viewRight; x++)
            {
                if (m_tiles[z][y][x] != nullptr)
                {
                    rTarget.draw(*m_tiles[z][y][x], states);
                }
            }
        }
    }

    if (m_displayGrid == true)
    {
        DrawGrid(rTarget, states);
    }
}

// Draw grid lines around Tiles
void Map::DrawGrid(sf::RenderTarget& rTarget, sf::RenderStates states) const
{
    sf::Vector2f viewPosition = rTarget.getView().getCenter();
    sf::Vector2f viewDimensions = rTarget.getView().getSize();

    m_horizGridLine.setSize(sf::Vector2f(viewDimensions.x, 2));
    m_vertGridLine.setSize(sf::Vector2f(2, viewDimensions.y));

    m_horizGridLine.setPosition(viewPosition.x - viewDimensions.x / 2,
                                viewPosition.y - viewDimensions.y / 2 -
                                static_cast<int>(viewPosition.y - viewDimensions.y / 2) % static_cast<int>(m_tileSize) - 1);
    if (m_horizGridLine.getPosition().y < -1)
    {
        m_horizGridLine.setPosition(m_horizGridLine.getPosition().x, -1);
    }
    if (m_horizGridLine.getPosition().x < 0)
    {
        m_horizGridLine.setPosition(0, m_horizGridLine.getPosition().y);
    }
    if (m_horizGridLine.getPosition().x + m_horizGridLine.getSize().x > GetBounds().x)
    {
        m_horizGridLine.setSize(sf::Vector2f(GetBounds().x - m_horizGridLine.getPosition().x, 2));
    }

    m_vertGridLine.setPosition(viewPosition.x - viewDimensions.x / 2 -
                               static_cast<int>(viewPosition.x - viewDimensions.x / 2) % static_cast<int>(m_tileSize) - 1,
                               viewPosition.y - viewDimensions.y / 2);
    if (m_vertGridLine.getPosition().x < -1)
    {
        m_vertGridLine.setPosition(-1, m_vertGridLine.getPosition().y);
    }
    if (m_vertGridLine.getPosition().y < 0)
    {
        m_vertGridLine.setPosition(m_vertGridLine.getPosition().x, 0);
    }
    if (m_vertGridLine.getPosition().y + m_vertGridLine.getSize().y > GetBounds().y)
    {
        m_vertGridLine.setSize(sf::Vector2f(2, GetBounds().y - m_vertGridLine.getPosition().y));
    }

    while (m_horizGridLine.getPosition().y <= viewPosition.y + viewDimensions.y / 2 &&
           m_horizGridLine.getPosition().y <= GetBounds().y)
    {
        rTarget.draw(m_horizGridLine);
        m_horizGridLine.setPosition(m_horizGridLine.getPosition() + sf::Vector2f(0, m_tileSize));
    }
    while (m_vertGridLine.getPosition().x <= viewPosition.x + viewDimensions.x / 2 &&
           m_vertGridLine.getPosition().x <= GetBounds().x)
    {
        rTarget.draw(m_vertGridLine);
        m_vertGridLine.setPosition(m_vertGridLine.getPosition() + sf::Vector2f(m_tileSize, 0));
    }
}

void Map::Update()
{
    
}

// Loads the Map from a save file
bool Map::Load(const std::string& filename)
{
    // First delete all elements of the vector (necessary when changing level)
    Clear();

    std::ifstream inf;
    inf.open(FileManager::ResourcePath() + filename);
    if (inf.is_open())
    {
        std::cout << "Loading Map...\n";
        inf >> m_indexDimensions.x >> m_indexDimensions.y >> m_tileSize;
        std::cout << "Dimensions:\t" << m_indexDimensions.x << 'x' << m_indexDimensions.y << '\n';
        std::cout << "TileSize:\t" << m_tileSize << '\n';

        // Vector resizing
        for (unsigned int z = 0; z < m_layerCount; z++)
        {
            m_tiles[z].resize(m_indexDimensions.y);
            for (unsigned int y = 0; y < m_indexDimensions.y; y++)
            {
                m_tiles[z][y].resize(m_indexDimensions.x);
            }
        }

        // Vector assigning
        std::cout << "Tile map:\n";
        for (unsigned int z = 0; z < m_layerCount; z++)
        {
            std::cout << '#' << z << ": \n";
            inf.ignore(); // Ignore the \n
            if (inf.get() == '-') // Empty layer
            {
                for (unsigned int y = 0; y < m_indexDimensions.y; y++)
                {
                    for (unsigned int x = 0; x < m_indexDimensions.x; x++)
                    {
                        m_tiles[z][y][x] = nullptr;
                    }
                }
                std::cout << "(empty)\n";
                continue;
            }
            else
            {
                inf.unget();
            }

            for (unsigned int y = 0; y < m_indexDimensions.y; y++)
            {
                bool reachedSemiColon = false;
                for (unsigned int x = 0; x < m_indexDimensions.x; x++)
                {
                    // If a semicolon has not been reached, read the next group of characters
                    if (reachedSemiColon == false)
                    {
                        std::string input;
                        inf >> input;
                        if (input == ";")
                        {
                            reachedSemiColon = true;
                        }
                        else
                        {
                            if (input.empty() ||
                                input.size() > 3 ||
                                std::find_if(input.cbegin(), input.cend(), [](char c)
                                {
                                    return std::isdigit(c) == false;
                                }) != input.cend())
                            {
                                std::cout << "\nMap error: Parsing tile type failed in file: \"" << filename << "\".\n"
                                             "Map loading failed.\n\n";
                                return false;
                            }

                            int type = std::stoi(input);
                            AddTile(static_cast<TileType>(type), sf::Vector2u(x, y), static_cast<MapLayer>(z), false);
                        }
                    }
                    // If a semicolon has been reached (possibly in this exact loop), set this index to a null pointer
                    if (reachedSemiColon == true)
                    {
                        m_tiles[z][y][x] = nullptr;
                    }

                    if (m_tiles[z][y][x] != nullptr)
                    {
                        std::cout << static_cast<int>(m_tiles[z][y][x]->GetTileType()) << ' ';
                    }
                    else
                    {
                        std::cout << "--- ";
                    }
                }
                std::cout << '\n';
            }
        }

        inf.close();
        std::cout << "Map successfully loaded.\n\n";
        return true;
    }
    else
    {
        std::cout << "Map error: Unable to open \"" << filename << "\".\n"
                     "Map loading failed.\n\n";
        return false;
    }
}

// Saves the Map to a save file
bool Map::Save(const std::string& filename)
{
    std::ofstream outf(FileManager::ResourcePath() + filename);
    if (outf.is_open())
    {
        std::cout << "Saving Map...\n";
        outf << m_indexDimensions.x << '\n' << m_indexDimensions.y << '\n' << m_tileSize << '\n';
        std::cout << "Dimensions:\t" << m_indexDimensions.x << 'x' << m_indexDimensions.y << '\n';
        std::cout << "TileSize:\t" << m_tileSize << '\n';

        ClearLayer(MapLayer::Overlay); // Skip saving overlay

        for (unsigned int z = 0; z < m_layerCount; z++)
        {
            std::string layerOutput;
            bool emptyLayer = true;
            for (unsigned int y = 0; y < m_indexDimensions.y; y++)
            {
                for (unsigned int x = 0; x < m_indexDimensions.x; x++)
                {
                    if (m_tiles[z][y][x] == nullptr)
                    {
                        layerOutput += "000";
                    }
                    else
                    {
                        layerOutput += std::to_string(static_cast<int>(m_tiles[z][y][x]->GetTileType()));
                        emptyLayer = false;
                    }
                    if (x + 1 < m_indexDimensions.x)
                    {
                        layerOutput += ' ';
                    }
                }
                layerOutput += '\n';
            }

            if (emptyLayer == false)
            {
                outf << layerOutput;
            }
            else
            {
                outf << "-\n";
            }
        }

        outf.close();
        std::cout << "Map successfully saved.\n\n";
        return true;
    }
    else
    {
        std::cout << "Map error: Unable to save \"" << filename << "\".\n"
                     "Map saving failed.\n\n";
        return false;
    }
}

// For converting a world coord to a Tile index
sf::Vector2u Map::CoordsToTileIndex(const sf::Vector2f& position) const
{
    return sf::Vector2u(position.x / m_tileSize, position.y / m_tileSize);
}

// For converting a Tile index to a world coord
sf::Vector2f Map::TileIndexToCoords(const sf::Vector2u& position) const
{
    return sf::Vector2f(position.x * m_tileSize, position.y * m_tileSize);
}

// Updates a Tile's texture according to surrounding Tiles
void Map::UpdateTileTexture(const sf::Vector2u& tileIndex, MapLayer layer)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    unsigned int x = tileIndex.x;
    unsigned int y = tileIndex.y;
    unsigned int z = static_cast<unsigned int>(layer);

    // If index is outside of Map bounds
    if (x >= m_indexDimensions.x || y >= m_indexDimensions.y)
    {
        return;
    }

    bool topLeftEmpty = true;
    bool topEmpty = true;
    bool topRightEmpty = true;
    bool rightEmpty = true;
    bool bottomRightEmpty = true;
    bool bottomEmpty = true;
    bool bottomLeftEmpty = true;
    bool leftEmpty = true;

    if (m_tiles[z][y][x] != nullptr && static_cast<int>(m_tiles[z][y][x]->GetTileType()) >= 100 && static_cast<int>(m_tiles[z][y][x]->GetTileType()) <= 136)
    {
        if (x > 0)
        {
            if (y > 0) topLeftEmpty = (m_tiles[z][y - 1][x - 1] == nullptr || m_tiles[z][y - 1][x - 1]->IsSolid() == false);
            else topLeftEmpty = false;

            if (y < m_indexDimensions.y - 1) bottomLeftEmpty = (m_tiles[z][y + 1][x - 1] == nullptr || m_tiles[z][y + 1][x - 1]->IsSolid() == false);
            else bottomLeftEmpty = false;

            leftEmpty = (m_tiles[z][y][x - 1] == nullptr || m_tiles[z][y][x - 1]->IsSolid() == false);
        }
        else
        {
            topLeftEmpty = false;
            bottomLeftEmpty = false;
            leftEmpty = false;
        }

        if (x < m_indexDimensions.x - 1)
        {
            if (y > 0) topRightEmpty = (m_tiles[z][y - 1][x + 1] == nullptr || m_tiles[z][y - 1][x + 1]->IsSolid() == false);
            else topRightEmpty = false;

            if (y < m_indexDimensions.y - 1) bottomRightEmpty = (m_tiles[z][y + 1][x + 1] == nullptr || m_tiles[z][y + 1][x + 1]->IsSolid() == false);
            else bottomRightEmpty = false;

            rightEmpty = (m_tiles[z][y][x + 1] == nullptr || m_tiles[z][y][x + 1]->IsSolid() == false);
        }
        else
        {
            topRightEmpty = false;
            bottomRightEmpty = false;
            rightEmpty = false;
        }

        if (y > 0)
        {
            topEmpty = (m_tiles[z][y - 1][x] == nullptr || m_tiles[z][y - 1][x]->IsSolid() == false);
        }
        else
        {
            topEmpty = false;
        }

        if (y < m_indexDimensions.y - 1)
        {
            bottomEmpty = (m_tiles[z][y + 1][x] == nullptr || m_tiles[z][y + 1][x]->IsSolid() == false);
        }
        else
        {
            bottomEmpty = false;
        }
        
        if (topEmpty == true &&
            leftEmpty == true && rightEmpty == false &&
            bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassTopLeftSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomEmpty == false && ((bottomRightEmpty == false && bottomLeftEmpty == false) || (bottomLeftEmpty == true && bottomRightEmpty == true)))
        {
            AddTile(TileType::GrassTopSide, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomLeftEmpty == false && bottomEmpty == false)
        {
            AddTile(TileType::GrassTopRightSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false && ((topRightEmpty == false && bottomRightEmpty == false) || (topRightEmpty == true && bottomRightEmpty == true)) &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == false)
        {
            AddTile(TileType::GrassLeftSide, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false && topRightEmpty == false &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomLeftEmpty == false && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassNoSides, sf::Vector2u(x, y), layer);
        }
        else if (((topLeftEmpty == false && bottomLeftEmpty == false) || (topLeftEmpty == true && bottomLeftEmpty == true)) && topEmpty == false  &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomEmpty == false)
        {
            AddTile(TileType::GrassRightSide, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false && topRightEmpty == false&&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotLeftSide, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false && ((topRightEmpty == false && topLeftEmpty == false) || (topRightEmpty == true && topLeftEmpty == true)) &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotSide, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotRightSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == true && rightEmpty == true &&
                 bottomEmpty == false)
        {
            AddTile(TileType::GrassTopLeftRightSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false  &&
                 leftEmpty == true && rightEmpty == true &&
                 bottomEmpty == false)
        {
            AddTile(TileType::GrassLeftRightSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false  &&
                 leftEmpty == true && rightEmpty == true &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotLeftRightSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassTopBotLeftSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassTopBotSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassTopBotRightSides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == true && rightEmpty == true &&
                 bottomEmpty == true)
        {
            AddTile(TileType::Grass4Sides, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassTopLeftSidesCorner3, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == false && bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassTopSideCorner3, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == true && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassTopSideCorner4, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == true  &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomLeftEmpty == true && bottomEmpty == false)
        {
            AddTile(TileType::GrassTopRightSidesCorner4, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false  && topRightEmpty == false &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassLeftSideCorner3, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false  && topRightEmpty == false &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == false && bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassNoSidesCorner3, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false  && topRightEmpty == false &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == true && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassNoSidesCorner4, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false  &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomLeftEmpty == true && bottomEmpty == false)
        {
            AddTile(TileType::GrassRightSideCorner4, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false  && topRightEmpty == true &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassLeftSideCorner2, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false && topRightEmpty == true &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomRightEmpty == false && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassNoSidesCorner2, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false && topRightEmpty == false &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == false && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassNoSidesCorner1, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomLeftEmpty == false && bottomEmpty == false)
        {
            AddTile(TileType::GrassRightSideCorner1, sf::Vector2u(x, y), layer);
        }
        else if (topEmpty == false && topRightEmpty == true &&
                 leftEmpty == true && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotLeftSidesCorner2, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false && topRightEmpty == true &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotSideCorner2, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false && topRightEmpty == false &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotSideCorner1, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false &&
                 leftEmpty == false && rightEmpty == true &&
                 bottomEmpty == true)
        {
            AddTile(TileType::GrassBotRightSidesCorner1, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false && topRightEmpty == true &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == true && bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassNoSides4Corners, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false && topRightEmpty == true &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == false && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassNoSidesCorners12, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false && topRightEmpty == false &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == true && bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassNoSidesCorners34, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == true && topEmpty == false && topRightEmpty == false &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == true && bottomEmpty == false && bottomRightEmpty == false)
        {
            AddTile(TileType::GrassNoSidesCorners14, sf::Vector2u(x, y), layer);
        }
        else if (topLeftEmpty == false && topEmpty == false && topRightEmpty == true &&
                 leftEmpty == false && rightEmpty == false &&
                 bottomLeftEmpty == false && bottomEmpty == false && bottomRightEmpty == true)
        {
            AddTile(TileType::GrassNoSidesCorners23, sf::Vector2u(x, y), layer);
        }
        else
        {
            AddTile(TileType::GrassNoSides, sf::Vector2u(x, y), layer);
        }
    }
}

// Creates a new Tile at the specified index
void Map::AddTile(TileType tileType, const sf::Vector2u& tileIndex, MapLayer layer, bool updateTextures)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    unsigned int x = tileIndex.x;
    unsigned int y = tileIndex.y;
    unsigned int z = static_cast<unsigned int>(layer);

    // If index is outside of Map bounds
    if (x >= m_indexDimensions.x || y >= m_indexDimensions.y)
    {
        return;
    }

    // Delete existing Tile if found
    if (m_tiles[z][y][x] != nullptr)
    {
        delete m_tiles[z][y][x];
    }

    switch (tileType)
    {
        default:
            m_tiles[z][y][x] = nullptr;
            break;
        case TileType::GrassTopLeftSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopLeftSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopSide:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopSide"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopRightSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopRightSides"), tileType);\
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassLeftSide:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassLeftSide"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassRightSide:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassRightSide"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotLeftSide:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotLeftSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotSide:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotSide"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotRightSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotRightSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopLeftRightSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopLeftRightSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassLeftRightSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassLeftRightSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotLeftRightSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotLeftRightSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopBotLeftSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopBotLeftSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopBotSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopBotSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopBotRightSides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopBotRightSides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::Grass4Sides:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grass4Sides"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopLeftSidesCorner3:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopLeftSidesCorner3"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopSideCorner3:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopSideCorner3"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopSideCorner4:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopSideCorner4"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassTopRightSidesCorner4:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassTopRightSidesCorner4"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassLeftSideCorner3:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassLeftSideCorner3"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorner3:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorner3"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorner4:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorner4"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassRightSideCorner4:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassRightSideCorner4"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassLeftSideCorner2:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassLeftSideCorner2"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorner2:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorner2"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorner1:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorner1"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassRightSideCorner1:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassRightSideCorner1"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotLeftSidesCorner2:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotLeftSidesCorner2"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotSideCorner2:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotSideCorner2"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotSideCorner1:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotSideCorner1"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassBotRightSidesCorner1:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassBotRightSidesCorner1"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSides4Corners:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSides4Corners"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorners12:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorners12"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorners34:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorners34"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorners14:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorners14"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::GrassNoSidesCorners23:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("grassNoSidesCorners23"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::Wood:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("wood"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::Ladder:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("ladder"), tileType);
            m_tiles[z][y][x]->SetSolid(false);
            break;
        case TileType::LadderTop:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("ladder"), tileType);
            m_tiles[z][y][x]->SetSolid(true);
            break;
        case TileType::Vine:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("vine"), tileType);
            m_tiles[z][y][x]->SetSolid(false);
            break;
        case TileType::Post:
            m_tiles[z][y][x] = new Tile(m_resourceManager.GetTexture("post"), tileType);
            m_tiles[z][y][x]->SetSolid(false);
            break;
    }
    
    if (m_tiles[z][y][x] != nullptr)
    {
        m_tiles[z][y][x]->SetPosition(sf::Vector2f(tileIndex.x * m_tileSize, tileIndex.y * m_tileSize));
        m_tiles[z][y][x]->SetDimensions(sf::Vector2f(m_tileSize, m_tileSize));
        m_tiles[z][y][x]->SetColor(m_layerColors[z]);
    }

    if (updateTextures == true)
    {
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                UpdateTileTexture(sf::Vector2u(x + i, y + j), layer);
            }
        }
    }
}

// Creates a new range of Tiles at the specified index
void Map::AddTileRange(TileType tileType, const sf::Vector2u& tileIndex, const sf::Vector2u& range, MapLayer layer, bool updateTextures)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    for (unsigned int y = 0; y < range.y; y++)
    {
        for (unsigned int x = 0; x < range.x; x++)
        {
            AddTile(tileType, tileIndex + sf::Vector2u(x, y), layer);

        }
    }

    if (updateTextures == true)
    {
        for (int y = -1; y <= static_cast<int>(range.y); y++)
        {
            for (int x = -1; x <= static_cast<int>(range.x); x++)
            {
                UpdateTileTexture(sf::Vector2u(tileIndex.x + x, tileIndex.y + y), layer);
            }
        }
    }
}

// Deletes the Tile at the specified index
void Map::RemoveTile(const sf::Vector2u& tileIndex, MapLayer layer, bool updateTextures)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    unsigned int x = tileIndex.x;
    unsigned int y = tileIndex.y;
    unsigned int z = static_cast<unsigned int>(layer);

    // If index is outside of Map bounds
    if (x >= m_indexDimensions.x || y >= m_indexDimensions.y)
    {
        return;
    }
    
    // Replace existing Tile if found
    if (m_tiles[z][y][x] != nullptr)
    {
        delete m_tiles[z][y][x];
        m_tiles[z][y][x] = nullptr;
    }

    if (updateTextures == true)
    {
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                UpdateTileTexture(sf::Vector2u(x + i, y + j), layer);
            }
        }
    }
}

// Deletes the range of Tiles at the specified index
void Map::RemoveTileRange(const sf::Vector2u& tileIndex, const sf::Vector2u& range, MapLayer layer, bool updateTextures)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    for (unsigned int y = 0; y < range.y; y++)
    {
        for (unsigned int x = 0; x < range.x; x++)
        {
            RemoveTile(tileIndex + sf::Vector2u(x, y), layer);
        }
    }

    if (updateTextures == true)
    {
        for (int y = -1; y <= static_cast<int>(range.y); y++)
        {
            for (int x = -1; x <= static_cast<int>(range.x); x++)
            {
                UpdateTileTexture(sf::Vector2u(tileIndex.x + x, tileIndex.y + y), layer);
            }
        }
    }
}

// Resizes the Map to the specified index dimensions
void Map::Resize(const sf::Vector2u& indexDimensions)
{
    if (indexDimensions.x == 0 || indexDimensions.y == 0)
    {
        return;
    }

    // Max dimensions
    sf::Vector2u newIndexDimensions = sf::Vector2u(fmin(indexDimensions.x, m_maxDimensions.x), fmin(indexDimensions.y, m_maxDimensions.y));

    for (unsigned int z = 0; z < m_layerCount; z++)
    {
        // Delete excess Tiles on the Y axis
        for (unsigned int y = newIndexDimensions.y; y < m_indexDimensions.y; y++)
        {
            for (unsigned int x = 0; x < m_indexDimensions.x; x++)
            {
                delete m_tiles[z][y][x];
            }
        }
        m_tiles[z].resize(newIndexDimensions.y);

        // Delete excess Tiles on the X axis
        for (unsigned int y = 0; y < newIndexDimensions.y; y++)
        {
            for (unsigned int x = newIndexDimensions.x; x < m_indexDimensions.x; x++)
            {
                delete m_tiles[z][y][x];
            }
            m_tiles[z][y].resize(newIndexDimensions.x);
        }

        // Set new Tiles on the Y axis to nullptr
        for (unsigned int y = m_indexDimensions.y; y < newIndexDimensions.y; y++)
        {
            for (unsigned int x = 0; x < newIndexDimensions.x; x++)
            {
                m_tiles[z][y][x] = nullptr;
            }
        }
        // Set new Tiles on the X axis to nullptr
        for (unsigned int y = 0; y < newIndexDimensions.y; y++)
        {
            for (unsigned int x = m_indexDimensions.x; x < newIndexDimensions.x; x++)
            {
                m_tiles[z][y][x] = nullptr;
            }
        }
    }

    m_indexDimensions = newIndexDimensions;
}

// Removes all Tiles by deleting them and setting them to nullptr, but conserves the Map's index dimensions
void Map::Clear()
{
    for (unsigned int z = 0; z < m_layerCount; z++)
    {
        for (unsigned int y = 0; y < m_indexDimensions.y; y++)
        {
            for (unsigned int x = 0; x < m_indexDimensions.x; x++)
            {
                delete m_tiles[z][y][x];
                m_tiles[z][y][x] = nullptr;
            }
        }
    }
}

// Removes all Tiles on a Layer by deleting them and setting them to nullptr
void Map::ClearLayer(MapLayer layer)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    for (unsigned int y = 0; y < m_indexDimensions.y; y++)
    {
        for (unsigned int x = 0; x < m_indexDimensions.x; x++)
        {
            delete m_tiles[static_cast<unsigned int>(layer)][y][x];
            m_tiles[static_cast<unsigned int>(layer)][y][x] = nullptr;
        }
    }
}

// Sets a layer's Tiles' color
void Map::SetLayerColor(sf::Color color, MapLayer layer)
{
    if (layer == MapLayer::Count)
    {
        return;
    }

    unsigned int z = static_cast<unsigned int>(layer);

    m_layerColors[z] = color;

    for (unsigned int y = 0; y < m_indexDimensions.y; y++)
    {
        for (unsigned int x = 0; x < m_indexDimensions.x; x++)
        {
            if (m_tiles[z][y][x] != nullptr)
            {
                m_tiles[z][y][x]->SetColor(m_layerColors[z]);
            }
        }
    }
}

// Returns Map dimensions, in world coords
sf::Vector2u Map::GetBounds() const
{
    return sf::Vector2u(m_indexDimensions.x * m_tileSize, m_indexDimensions.y * m_tileSize);
}

// Returns true if the Map is 0x0
bool Map::IsNull() const
{
    return (m_indexDimensions == sf::Vector2u(0, 0));
}

// Returns a pointer to a const Tile at given coords
const Tile* Map::GetKTilePtr(const sf::Vector2u& index, MapLayer layer) const
{
    if (layer == MapLayer::Count || index.x >= m_indexDimensions.x || index.y >= m_indexDimensions.y)
    {
        return nullptr;
    }
    return m_tiles[static_cast<int>(layer)][index.y][index.x];
}

// Returns a pointer to a Tile at given coords
Tile* Map::GetTilePtr(const sf::Vector2u& index, MapLayer layer) const
{
    if (layer == MapLayer::Count || index.x >= m_indexDimensions.x || index.y >= m_indexDimensions.y)
    {
        return nullptr;
    }
    return m_tiles[static_cast<int>(layer)][index.y][index.x];
}
