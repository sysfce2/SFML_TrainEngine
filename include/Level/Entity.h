#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "Level/Map.h"
#include "Level/Tile.h"
#include "Misc/AnimatedSprite.h"

enum class EntityType
{
    Player = 000
};

enum class EntityState
{
    Still,
    Running,
    Climbing,
    Jumping,
    Falling
};

class Entity : public sf::Drawable
{
private:
    EntityType m_entityType;
    EntityState m_state;

    // Graphics variables
    sf::Sprite m_defaultSprite;
    std::unordered_map<EntityState, AnimatedSprite> m_animatedSprites;

    // Debug information
    sf::RectangleShape m_collisionBox;
    sf::RectangleShape m_tileReactionDot;
    bool m_isDebugBoxVisible;

    // Physics variables
    sf::Vector2f m_position;
    sf::Vector2f m_previousPosition;
    sf::Vector2f m_dimensions;
    sf::Vector2f m_velocity;

    const sf::Vector2f m_maxVelocity;

    float m_acceleration;
    float m_deceleration;

    bool m_isTileCollideable;
    bool m_isEntityCollideable;

    bool m_isFacingRight;
    bool m_isGravityApplied;
    bool m_isOnGround;

    // Input controls
    sf::Vector2f m_inputDirection;
    bool m_isPressingUp;

    // Movement settings
    float m_jumpForce;
    float m_defaultClimbSpeed;
    float m_defaultDescentSpeed;

    // Collisions and interactions
    void TileCollision(const Tile* pTile);
    void EntityCollision(const Entity* pEntity);
    void TileReaction(Tile* pTile);
    void EntityReaction(Entity* pEntity);

    // TileCollision functions (overrideable)
    virtual void StandardCollision(const Tile* pTile);
    virtual void LadderTopCollision(const Tile* pTile);

    // EntityCollision functions (overrideable)
    // ---

    // TileReaction functions (overrideable)
    // ---

    // EntityReaction functions (overrideable)
    virtual void PlayerReaction(Entity* pEntity) {}

    // Movement
    void MoveLeft();
    void MoveRight();
    void Jump();
    void Climb(float factor = 1);

protected:
    Map& m_rMap;
    std::vector<Entity*>& m_rEntities;

    // Functions
    virtual void draw(sf::RenderTarget& rTarget, sf::RenderStates states) const override;
    void ApplyDeceleration();
    void ApplyGravity();
    void MaxVelocityCap();
    void MapEdgeCollision(bool isHorizCollisionEnabled = true, bool isVertCollisionEnabled = true);
    void PerformCollisions();

    // Setters
    void SetHorizontalInputDirection(float horizInputDirection) { m_inputDirection.x = horizInputDirection; }
    void SetVerticalInputDirection(float vertInputDirection) { m_inputDirection.y = vertInputDirection; }
    void SetPressingUp(bool isPressingUp) { m_isPressingUp = isPressingUp; }

    // Getters
    bool IsOnGround() const { return m_isOnGround; }

    float GetHorizontalInputDirection() const { return m_inputDirection.x; }
    float GetVerticalInputDirection() const { return m_inputDirection.y; }

    float GetJumpForce() const { return m_jumpForce; }
    float GetDefaultClimbSpeed() const { return m_defaultClimbSpeed; }
    float GetDefaultDescentSpeed() const { return m_defaultDescentSpeed; }

public:
    // Constructor and destructor
    Entity(Map& rMap, std::vector<Entity*>& rEntities, EntityType entityType, const sf::Vector2f& position, const sf::Vector2f& dimensions,
           const sf::Vector2f& maxVelocity = {10, 64}, float acceleration = 1, float deceleration = 3, float jumpForce = 18,
           bool isGravityApplied = true, bool isTileCollideable = false, bool isEntityCollideable = false);
    virtual ~Entity() {}

    // Functions
    virtual void HandleInput() {}
    virtual void Update();
    virtual void Interpolate(float lag);

    void SetStateAnimation(EntityState targetState, const AnimatedSprite& animatedSprite, float frameDuration, bool isLoopingEnabled = true);
    void SetStateAnimation(EntityState targetState, AnimatedSprite&& animatedSprite, float frameDuration, bool isLoopingEnabled = true);

    // Setters
    void SetDebugBoxVisible(bool isDebugBoxVisible) { m_isDebugBoxVisible = isDebugBoxVisible; }

    void SetPosition(const sf::Vector2f& position) { m_position = position; }
    void SetHorizPosition(float horizPosition) { m_position.x = horizPosition; }
    void SetVertPosition(float vertPosition) { m_position.y = vertPosition; }

    void SetDimensions(const sf::Vector2f& dimensions) { m_dimensions = dimensions; }

    void SetVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }
    void SetHorizVelocity(float horizVelocity) { m_velocity.x = horizVelocity; }
    void SetVertVelocity(float vertVelocity) { m_velocity.y = vertVelocity; }

    void SetDefaultSpriteTexture(const sf::Texture& texture);

    // Getters
    EntityType GetEntityType() const { return m_entityType; }
    static std::string GetEntityTypeString(EntityType entityType);
    static std::vector<std::string> GetTextureNames(EntityType entityType);

    const sf::Vector2f& GetPosition() const { return m_position; }
    const sf::Vector2f& GetDimensions() const { return m_dimensions; }
    const sf::Vector2f& GetVelocity() const { return m_velocity; }

    float GetLeftPixelPosition() const;
    float GetRightPixelPosition() const;
    float GetTopPixelPosition() const;
    float GetBottomPixelPosition() const;
};

#endif // ENTITY_H
