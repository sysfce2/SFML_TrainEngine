#include "Entity.h"
#include <math.h>

float Entity::s_gravity = 1.3;

Entity::Entity(Map& rMap, std::vector<Entity*>& rEntities, EntityType entityType,
               const sf::Vector2f& position, const sf::Vector2f& dimensions, const sf::Vector2f& maxVelocity,
               float acceleration, float deceleration, bool applyGravity,
               bool isTileCollideable, bool isEntityCollideable, unsigned int collisionPriority)
    : m_rMap(rMap),
      m_rEntities(rEntities),
      m_entityType(entityType),
      m_state(EntityState::Still),
      m_collisionBox(dimensions),
      m_displayCollisionBox(false),
      m_position(position),
      m_previousPosition(position),
      m_dimensions(dimensions),
      m_velocity(sf::Vector2f(0, 0)),
      m_maxVelocity(maxVelocity),
      m_acceleration(acceleration),
      m_deceleration(deceleration),
      m_isTileCollideable(isTileCollideable),
      m_isEntityCollideable(isEntityCollideable),
      m_collisionPriority(collisionPriority),
      m_isGoingRight(true),
      m_applyGravity(applyGravity),
      m_isOnGround(false)
{
    m_collisionBox.setOrigin(m_dimensions / 2.0f);
    m_collisionBox.setPosition(m_position);
    m_collisionBox.setFillColor(sf::Color(255, 100, 100, 100));
    m_collisionBox.setOutlineColor(sf::Color::Black);
    m_collisionBox.setOutlineThickness(-1);
}

void Entity::draw(sf::RenderTarget& rTarget, sf::RenderStates states) const
{
    auto it = m_animatedSprites.find(m_state);
    if (it != m_animatedSprites.cend())
    {
        rTarget.draw(it->second, states);
    }
    else
    {
        rTarget.draw(m_defaultSprite, states);
    }
    if (m_displayCollisionBox == true)
    {
        rTarget.draw(m_collisionBox, states);
    }
}

// Apply deceleration
void Entity::ApplyDeceleration()
{
    if (m_velocity.x >= m_deceleration) m_velocity.x -= m_deceleration;
    else if (m_velocity.x <= -m_deceleration) m_velocity.x += m_deceleration;
    else m_velocity.x = 0;
}

// Apply gravity to the Entity
void Entity::ApplyGravity()
{
    if (m_applyGravity == true) m_velocity.y += s_gravity;
}

// Cap the Entity's velocity
void Entity::MaxVelocityCap()
{
    if (m_velocity.x >= m_maxVelocity.x) m_velocity.x = m_maxVelocity.x;
    else if (m_velocity.x <= -m_maxVelocity.x) m_velocity.x = -m_maxVelocity.x;
    if (m_velocity.y >= m_maxVelocity.y) m_velocity.y = m_maxVelocity.y;
    else if (m_velocity.y <= -m_maxVelocity.y) m_velocity.y = -m_maxVelocity.y;
}

// Apply collision with the edges of the Map
void Entity::MapEdgeCollision(bool applyHorizCollision, bool applyVertCollision)
{
    // If Map is null
    if (m_rMap.IsNull())
    {
        return;
    }
    
    if (applyHorizCollision == true)
    {
        if (m_position.x - m_dimensions.x / 2 + m_velocity.x <= 0)
        {
            m_position.x = 0 + m_dimensions.x / 2;
            ResetHorizVelocity();
        }
        else if (m_position.x + m_dimensions.x / 2 + m_velocity.x >= m_rMap.GetBounds().x)
        {
            m_position.x = m_rMap.GetBounds().x - m_dimensions.x / 2;
            ResetHorizVelocity();
        }
    }
    if (applyVertCollision == true)
    {
        if (m_position.y - m_dimensions.y / 2 + m_velocity.y <= 0)
        {
            m_position.y = 0 + m_dimensions.y / 2;
            ResetVertVelocity();
            m_isOnGround = false;
        }
        else if (m_position.y + m_dimensions.y / 2 + m_velocity.y >= m_rMap.GetBounds().y)
        {
            m_position.y = m_rMap.GetBounds().y - m_dimensions.y / 2;
            ResetVertVelocity();
            m_isOnGround = true;
        }
    }
}

// Perform collision checks and resolutions with Tiles, Map edge, and Entities
void Entity::PerformCollisions()
{
    // Determine the range of positions where the Entity could be in the next tick
    float biggestAxis = fmax(m_dimensions.x, m_dimensions.y);
    float highestVelocity = fmax(m_velocity.x, m_velocity.y);
    float range = (biggestAxis + highestVelocity) / m_rMap.GetTileSize();
    
    // Collision with Tiles
    if (m_isTileCollideable == true)
    {
        // Loop through Tiles in the same direction as the Entity's velocity to fix collision problems
        sf::Vector2u positionIndex = m_rMap.CoordsToTileIndex(m_position);
        if (m_velocity.x >= 0)
        {
            for (int i = -range - 1; i <= range + 1; i++)
            {
                if (m_velocity.y >= 0)
                {
                    for (int j = -range - 1; j <= range + 1; j++)
                    {
                        TileCollision(m_rMap.GetKTilePtr(sf::Vector2u(positionIndex.x + i, positionIndex.y + j), MapLayer::Solid));
                    }
                }
                else
                {
                    for (int j = range + 1; j >= -range - 1; j--)
                    {
                        TileCollision(m_rMap.GetKTilePtr(sf::Vector2u(positionIndex.x + i, positionIndex.y + j), MapLayer::Solid));
                    }
                }
            }
        }
        else
        {
            for (int i = range + 1; i >= -range - 1; i--)
            {
                if (m_velocity.y >= 0)
                {
                    for (int j = -range - 1; j <= range + 1; j++)
                    {
                        TileCollision(m_rMap.GetKTilePtr(sf::Vector2u(positionIndex.x + i, positionIndex.y + j), MapLayer::Solid));
                    }
                }
                else
                {
                    for (int j = range + 1; j >= -range - 1; j--)
                    {
                        TileCollision(m_rMap.GetKTilePtr(sf::Vector2u(positionIndex.x + i, positionIndex.y + j), MapLayer::Solid));
                    }
                }
            }
        }
    }
    
    MapEdgeCollision();
    
    // Collision with Entities
    if (m_isEntityCollideable == true)
    {
        for (auto& rEntity : m_rEntities)
        {
            if (rEntity != this)
            {
                EntityCollision(rEntity);
            }
        }
    }
}

// Apply collision with a const pointer to a const Tile
void Entity::TileCollision(const Tile* const pTile)
{
    // If the Tile is null
    if (pTile == nullptr)
    {
        return;
    }
    if (pTile->IsSolid())
    {
        switch (pTile->GetTileType())
        {
        default:
            StandardCollision(pTile);
            break;
        case TileType::LadderTop:
            LadderTopCollision(pTile);
            break;
        }
    }
}

// Collision used for Tiles that have collision for all four sides
void Entity::StandardCollision(const Tile* const pTile)
{
    sf::Vector2f tilePosition = pTile->GetPosition();
    sf::Vector2f tileDimensions = pTile->GetDimensions();
    // Check for Y-axis overlap
    if (m_position.y + m_dimensions.y / 2 + m_velocity.y >= tilePosition.y &&
        m_position.y - m_dimensions.y / 2 + m_velocity.y < tilePosition.y + tileDimensions.y)
    {
        if (m_position.x + m_dimensions.x / 2 > tilePosition.x &&
            m_position.x - m_dimensions.x / 2 < tilePosition.x + tileDimensions.x)
        {
            // If Entity will be above Tile
            if (m_velocity.y > 0)
            {
                m_position.y = tilePosition.y - m_dimensions.y / 2;
                m_isOnGround = true;
            }
            // If Entity will be under Tile
            else if (m_velocity.y < 0)
            {
                m_position.y = tilePosition.y + tileDimensions.y + m_dimensions.y / 2;
                m_isOnGround = false;
            }
            ResetVertVelocity();
        }
    }
    // Check for X-axis overlap
    if (m_position.x + m_dimensions.x / 2 + m_velocity.x >= tilePosition.x &&
        m_position.x - m_dimensions.x / 2 + m_velocity.x < tilePosition.x + tileDimensions.x)
    {
        if (m_position.y + m_dimensions.y / 2 > tilePosition.y &&
            m_position.y - m_dimensions.y / 2 < tilePosition.y + tileDimensions.y)
        {
            // If Entity will be on the left side of Tile
            if (m_velocity.x > 0)
            {
                m_position.x = tilePosition.x - m_dimensions.x / 2;
            }
            // If Entity will be on the right side of Tile
            else if (m_velocity.x < 0)
            {
                m_position.x = tilePosition.x + tileDimensions.x + m_dimensions.x / 2;
            }
            ResetHorizVelocity();
        }
    }
    
    // Check for corner overlap
    if (m_position.x + m_dimensions.x / 2 + m_velocity.x >= tilePosition.x &&
        m_position.x - m_dimensions.x / 2 + m_velocity.x < tilePosition.x + tileDimensions.x &&
        m_position.y + m_dimensions.y / 2 + m_velocity.y >= tilePosition.y &&
        m_position.y - m_dimensions.y / 2 + m_velocity.y < tilePosition.y + tileDimensions.y)
    {
        // If Entity is going right and down
        if (m_velocity.x > 0 && m_velocity.y > 0)
        {
            if (m_position.x + m_dimensions.x / 2 + m_velocity.x - tilePosition.x >= m_position.y + m_dimensions.y / 2 + m_velocity.y - tilePosition.y)
            {
                m_position.y = tilePosition.y - m_dimensions.y / 2;
                m_isOnGround = true;
                ResetVertVelocity();
            }
            else
            {
                m_position.x = tilePosition.x - m_dimensions.x / 2;
                ResetHorizVelocity();
            }
        }
        // If Entity is going left and down
        else if (m_velocity.x < 0 && m_velocity.y > 0)
        {
            if (tilePosition.x + tileDimensions.x - m_position.x - m_dimensions.x / 2 + m_velocity.x >= m_position.y + m_dimensions.y / 2 + m_velocity.y - tilePosition.y)
            {
                m_position.y = tilePosition.y - m_dimensions.y / 2;
                m_isOnGround = true;
                ResetVertVelocity();
            }
            else
            {
                m_position.x = tilePosition.x + tileDimensions.x + m_dimensions.x / 2;
                ResetHorizVelocity();
            }
        }
        // If Entity is going right and up
        else if (m_velocity.x > 0 && m_velocity.y < 0)
        {
            if (m_position.x - m_dimensions.x / 2 + m_velocity.x - tilePosition.x >= tilePosition.y + tileDimensions.y - m_position.y - m_dimensions.y / 2 + m_velocity.y)
            {
                m_position.y = tilePosition.y + tileDimensions.y + m_dimensions.y / 2;
                ResetVertVelocity();
            }
            else
            {
                m_position.x = tilePosition.x - m_dimensions.x / 2;
                ResetHorizVelocity();
            }
        }
        // If Entity is going left and up
        else if (m_velocity.x < 0 && m_velocity.y < 0)
        {
            if (tilePosition.x + tileDimensions.x - m_position.x - m_dimensions.x / 2 + m_velocity.x >= tilePosition.y + tileDimensions.y - m_position.y - m_dimensions.y / 2 + m_velocity.y)
            {
                m_position.y = tilePosition.y + tileDimensions.y + m_dimensions.y / 2;
                ResetVertVelocity();
            }
            else
            {
                m_position.x = tilePosition.x + tileDimensions.x + m_dimensions.x / 2;
                ResetHorizVelocity();
            }
        }
    }
}

// Collision used for LadderTop Tiles that have collision for all four sides
void Entity::LadderTopCollision(const Tile* const pTile)
{
    sf::Vector2f tilePosition = pTile->GetPosition();
    sf::Vector2f tileDimensions = pTile->GetDimensions();
    // If Entity is going downwards
    if (m_velocity.y >= 0)
    {
        // Check for x-axis overlap
        if (m_position.x + m_dimensions.x / 2 > tilePosition.x &&
            m_position.x - m_dimensions.x / 2 < tilePosition.x + tileDimensions.x)
        {
            // If in the current tick, the Entity's bottom side is above the LadderTop,
            // but it will be inside the LadderTop in the next tick
            if (m_position.y + m_dimensions.y / 2 <= tilePosition.y &&
                m_position.y + m_dimensions.y / 2 + m_velocity.y > tilePosition.y)
            {
                m_position.y = tilePosition.y - m_dimensions.y / 2;
                m_isOnGround = true;
                ResetVertVelocity();
            }
        }
    }
}

// Apply collision with a const pointer to a const Entity
void Entity::EntityCollision(const Entity* const pEntity)
{
    // TO DO
}

void Entity::Update()
{
    m_previousPosition = m_position;

    // Movement
    ApplyGravity();

    // Max velocity
    MaxVelocityCap();
    
    // For falling off a ledge
    m_isOnGround = false;
    
    // Collisions
    PerformCollisions();

    m_position += m_velocity;
    
    // Animations
    auto it = m_animatedSprites.find(m_state);
    if (it != m_animatedSprites.cend())
    {
        it->second.Update(!m_isGoingRight);
        it->second.Play();
    }
    
    m_collisionBox.setPosition(m_position);
}

void Entity::Interpolate(float lag)
{
    sf::Vector2f position(m_previousPosition + (m_position - m_previousPosition) * lag);
    auto it = m_animatedSprites.find(m_state);
    if (it != m_animatedSprites.cend())
    {
        it->second.SetPosition(position);
    }
    else
    {
        m_defaultSprite.setPosition(position);
    }
}

void Entity::AddAnimation(EntityState targetState, const AnimatedSprite& animatedSprite)
{
    auto it = m_animatedSprites.find(targetState);
    // If animation is already set to a state, overwrite it
    if (it != m_animatedSprites.end())
    {
        it->second = animatedSprite;
        return;
    }
    // Otherwise, add it to the map of animations
    m_animatedSprites.emplace(targetState, animatedSprite);

    m_animatedSprites.at(targetState).SetPosition(m_position);
}

void Entity::AddAnimation(EntityState targetState, AnimatedSprite&& animatedSprite)
{
    auto it = m_animatedSprites.find(targetState);
    // If animation is already set to a state, overwrite it
    if (it != m_animatedSprites.end())
    {
        it->second = std::move(animatedSprite);
        return;
    }
    // Otherwise, add it to the map of animations
    m_animatedSprites.emplace(targetState, std::move(animatedSprite));

    m_animatedSprites.at(targetState).SetPosition(m_position);
}

void Entity::SetPosition(const sf::Vector2f& position)
{
    m_position = position;
}

void Entity::SetDimensions(const sf::Vector2f& dimensions)
{
    m_dimensions = dimensions;
}

void Entity::ResetVelocity()
{
    m_velocity = sf::Vector2f(0, 0);
}

void Entity::ResetHorizVelocity()
{
    m_velocity.x = 0;
}

void Entity::ResetVertVelocity()
{
    m_velocity.y = 0;
}

void Entity::SetDefaultSpriteTexture(const sf::Texture& texture)
{
    m_defaultSprite.setTexture(texture);
    m_defaultSprite.setOrigin(static_cast<sf::Vector2f>(texture.getSize()) / 2.0f);
    m_defaultSprite.setPosition(m_position);
}

std::string Entity::GetEntityTypeString(EntityType entityType)
{
    static const std::unordered_map<EntityType, std::string> entityTypeStrings = {{EntityType::Player, "Player"}};
    
    const auto it = entityTypeStrings.find(entityType);
    if (it != entityTypeStrings.cend())
    {
        return it->second;
    }
    return "Unknown EntityType";
}

std::vector<std::string> Entity::GetTextureNames(EntityType entityType)
{
    static const std::unordered_map<EntityType, std::vector<std::string>> entityTextures = {{EntityType::Player, {"characterStill", "characterRunning", "characterClimbing", "characterJumping", "characterFalling"}}};

    const auto it = entityTextures.find(entityType);
    if (it != entityTextures.cend())
    {
        return it->second;
    }
    return {};
}

// Functions used for getting Entity's pixel position bounds

float Entity::GetLeftPixelPosition() const
{
    return m_position.x - m_dimensions.x / 2;
}

float Entity::GetRightPixelPosition() const
{
    return m_position.x + m_dimensions.x / 2 - 1; // Solves off-by-one error caused by pixel width
}

float Entity::GetTopPixelPosition() const
{
    return m_position.y - m_dimensions.y / 2;
}

float Entity::GetBottomPixelPosition() const
{
    return m_position.y + m_dimensions.y / 2 - 1; // Solves off-by-one error caused by pixel width
}
