#ifndef CAMERA_H
#define CAMERA_H

#include <SFML/Graphics.hpp>
#include "Level/Entity.h"

enum class CameraMode
{
    Static,
    Moving,
    Follow,
    Translate,
    SmoothTranslate
};

class Camera final
{
private:
    sf::View m_view;

    CameraMode m_mode;
    bool m_isBoundless; // If the Camera should be contained within the bounds

    sf::Vector2f m_bounds; // Area the Camera is allowed to travel in (generally Map bounds)

    sf::Vector2f m_position;
    sf::Vector2f m_previousPosition; // Previous position for interpolation
    sf::Vector2f m_moveOffset; // Position offset for free movement

    sf::Vector2f m_dimensions;
    sf::Vector2f m_previousDimensions; // Previous dimensions for interpolation
    sf::Vector2f m_targetDimensions; // Target dimensions for zoom lerp (lerp if not identical to m_dimensions)

    sf::Vector2f m_minDimensions;
    sf::Vector2f m_maxDimensions;

    float m_zoom;
    float m_zoomLerp;

    // For CameraMode::Follow
    const Entity* m_followedEntity;
    float m_followLerp;

    // For CameraMode::Translate
    sf::Vector2f m_startTranslationPosition;
    sf::Vector2f m_finalTranslationPosition;
    unsigned int m_ticksRemaining;
    unsigned int m_ticksTotal;

    // Functions
    void resizeToFitMinDimensions(sf::Vector2f& dimensions, const sf::Vector2f& minDimensions);
    void resizeToFitMaxDimensions(sf::Vector2f& dimensions, const sf::Vector2f& maxDimensions);
    void boundsCollision(sf::Vector2f& position, const sf::Vector2f& dimensions);

public:
    // Constructor
    Camera();

    // Functions
    void update();
    void interpolate(float lag);
    void setPosition(const sf::Vector2f& position);
    void move(const sf::Vector2f& offset);
    void setFollow(const Entity& followedEntity, bool snapOnSet = false);
    void setTranslate(const sf::Vector2f& startPosition, const sf::Vector2f& endPosition, unsigned int tickDuration,
                      bool isSlowDownSmooth = false);

    // Setters
    void setBoundless(bool isBoundless) { m_isBoundless = isBoundless; }
    void setBounds(const sf::Vector2f& bounds);
    void setDimensions(const sf::Vector2f& dimensions);
    void setMinDimensions(const sf::Vector2f& minDimensions);
    void setMaxDimensions(const sf::Vector2f& maxDimensions);
    void zoom(float zoom);
    void setZoom(float absoluteZoom);
    void setZoomLerp(float zoomLerp) { m_zoomLerp = zoomLerp; }
    void setFollowLerp(float followLerp) { m_followLerp = followLerp; }
    void setRotation(float angle) { m_view.setRotation(angle); }

    // Getters
    const sf::View& getView() const { return m_view; }
    CameraMode getMode() const { return m_mode; }
    bool isBoundless() const { return m_isBoundless; }
    const sf::Vector2f& getPosition() const { return m_view.getCenter(); }
    const sf::Vector2f& getDimensions() const { return m_view.getSize(); }
    float getZoom() const { return m_zoom; }
};

#endif // CAMERA_H
