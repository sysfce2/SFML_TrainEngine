#include "Gui/TextBox.h"
#include <cmath>

namespace
{
    const sf::Uint32 passwordChar = 8226;

    bool isCharacterAccepted(sf::Uint32 enteredChar)
    {
        return (enteredChar >= 32 && enteredChar <= 126) || enteredChar > 160;
    }
} // namespace

TextBox::TextBox(InputManager& inputManager, const sf::Font& font)
    : TextBox(inputManager, font, sf::Vector2f(0, 0), sf::Vector2f(100, 20))
{
}

TextBox::TextBox(InputManager& inputManager, const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& dimensions)
    : m_inputManager(inputManager)
    , m_position(position)
    , m_padding(6, 4)
    , m_opaquePaddingProportion(0.25)
    , m_startingOpacity(64)
    , m_endingOpacity(255)
    , m_hasFocus(false)
    , m_isReadOnly(false)
    , m_isDigitsOnly(false)
    , m_isPasswordModeEnabled(false)
    , m_cursorIndex(0)
    , m_selectionStartIndex(0)
    , m_isCursorVisible(true)
    , m_cursorTickCount(0)
    , m_cursorBlinkSpeed(30)
    , m_dragCursorProgress(0)
    , m_dragCursorSpeedDivider(20)
    , m_maxTextLength(256)
    , m_textCanExceedBox(true)
    , m_hideBoxLeft(sf::TriangleStrip, 6)
    , m_hideBoxRight(sf::TriangleStrip, 6)
{
    // Background color
    setBackgroundColor(sf::Color::White);

    // Outline settings
    setOutlineThickness(1);
    setOutlineColor(sf::Color(90, 90, 90));
    setOutlineColorFocused(sf::Color(36, 109, 226));
    if (m_hasFocus == true)
    {
        m_box.setOutlineColor(m_outlineColorFocused);
    }
    else
    {
        m_box.setOutlineColor(m_outlineColor);
    }

    // Text settings
    setText("");
    setPlaceholderText("");
    setFont(font);
    setTextColor(sf::Color::Black);
    setBackgroundTextColor(sf::Color(90, 90, 90));
    setCharacterSize(30);

    // Cursor settings
    setCursorColor(sf::Color::Black);
    setCursorThickness(1);

    // Selection settings
    setSelectionColor(sf::Color(36, 109, 226, 150));

    setDimensions(dimensions);
    setPosition(m_position);

    m_renderTexture.clear();
}

TextBox::~TextBox()
{
}

void TextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_box, states);
    if (m_cursorIndex != m_selectionStartIndex)
    {
        target.draw(m_selection, states);
    }
    target.draw(m_renderSprite, states);
    target.draw(m_hideBoxLeft, states);
    target.draw(m_hideBoxRight, states);
    if (m_hasFocus == true && m_isCursorVisible == true)
    {
        target.draw(m_cursor, states);
    }
    if (m_hasFocus == false && m_displayText.getString().isEmpty())
    {
        target.draw(m_backgroundText, states);
    }
}

void TextBox::drawTexture()
{
    m_renderTexture.clear(sf::Color::Transparent);
    m_renderTexture.draw(m_displayText);
    m_renderTexture.display();
}

// Check if the mouse is inside the box
bool TextBox::checkMousePosition() const
{
    sf::Vector2f mousePos = static_cast<sf::Vector2f>(m_inputManager.getWindowMousePosition());
    return mousePos.x > m_box.getPosition().x && mousePos.x < m_box.getPosition().x + m_box.getSize().x &&
           mousePos.y > m_box.getPosition().y && mousePos.y < m_box.getPosition().y + m_box.getSize().y;
}

void TextBox::centerText()
{
    m_displayText.setPosition(m_displayText.getPosition().x,
                              std::round(m_position.y +
                                         (m_box.getSize().y - m_displayText.getFont()->getLineSpacing(m_displayText.getCharacterSize())) /
                                             2));
    m_backgroundText.setPosition(m_box.getPosition().x + m_padding.x, m_displayText.getPosition().y);
}

void TextBox::setDisplayText(const sf::String& text)
{
    m_text = text;
    if (m_isPasswordModeEnabled == true)
    {
        sf::String passwordDisplayText = m_text;
        for (auto& displayChar : passwordDisplayText)
        {
            displayChar = passwordChar;
        }
        m_displayText.setString(passwordDisplayText);
    }
    else
    {
        m_displayText.setString(text);
    }
}

void TextBox::updateCursor()
{
    // If the left or right arrows are pressed
    if (m_inputManager.isKeyDescending(sf::Keyboard::Left, true) || m_inputManager.isKeyDescending(sf::Keyboard::Right, true))
    {
        controlCursorArrow();
        setTextPosition();
        drawTexture();
        setCursorPosition();
        if (m_selectionStartIndex != m_cursorIndex)
        {
            setSelectionBounds();
        }
    }
    if (m_inputManager.isMouseButtonHeld(sf::Mouse::Left) || m_inputManager.isMouseButtonDescending(sf::Mouse::Left))
    {
        if (m_inputManager.getWindowMousePosition().x > m_box.getPosition().x &&
            m_inputManager.getWindowMousePosition().x < m_box.getPosition().x + m_box.getSize().x)
        {
            if ((m_inputManager.isMouseButtonHeld(sf::Mouse::Left) && m_inputManager.detectedMouseMovedEvent()) ||
                m_inputManager.isMouseButtonDescending(sf::Mouse::Left))
            {
                controlCursorMouse();
            }
        }
        else
        {
            dragCursor();
        }
        setTextPosition();
        drawTexture();
        setCursorPosition();
        // Reset selection on mouse click
        if (m_inputManager.isMouseButtonDescending(sf::Mouse::Left))
        {
            resetSelection();
        }
        else
        {
            setSelectionBounds();
        }
    }
}

// Set the position of the cursor at its character position
void TextBox::setCursorPosition()
{
    if (m_displayText.getString().isEmpty())
    {
        m_cursor.setPosition(sf::Vector2f(m_box.getPosition().x + m_padding.x, m_cursor.getPosition().y));
    }
    else
    {
        m_cursor.setPosition(m_displayText.findCharacterPos(m_cursorIndex).x, m_cursor.getPosition().y);
    }
}

void TextBox::moveCursorRight()
{
    m_cursorIndex++;
    m_isCursorVisible = true;
    m_cursorTickCount = 0;
}

void TextBox::moveCursorLeft()
{
    m_cursorIndex--;
    m_isCursorVisible = true;
    m_cursorTickCount = 0;
}

// Control left and right arrows
// Shift selection is supported
void TextBox::controlCursorArrow()
{
    // If left arrow key is pressed
    if (m_inputManager.isKeyDescending(sf::Keyboard::Left, true))
    {
        // If there is a selection and shift and control are not held
        if (m_selectionStartIndex != m_cursorIndex && !m_inputManager.isShiftKeyHeld())
        {
            if (m_selectionStartIndex < m_cursorIndex)
            {
                m_cursorIndex = m_selectionStartIndex;
            }
            else
            {
                m_selectionStartIndex = m_cursorIndex;
            }
            m_isCursorVisible = true;
            m_cursorTickCount = 0;
        }
        // If there is no selection and the cursor has space to go left
        else if (m_cursorIndex > 0)
        {
            // Modifier key
            if (m_inputManager.isModifierKeyHeld())
            {
                // Loop through text
                moveCursorToPreviousSpace();
            }
            else
            {
                moveCursorLeft();
            }
            if (!m_inputManager.isShiftKeyHeld())
            {
                m_selectionStartIndex = m_cursorIndex;
            }
        }
    }
    // If the right arrow key is pressed
    else if (m_inputManager.isKeyDescending(sf::Keyboard::Right, true))
    {
        // If there is a selection and shift and control are not held
        if (m_selectionStartIndex != m_cursorIndex && !m_inputManager.isShiftKeyHeld())
        {
            if (m_selectionStartIndex < m_cursorIndex)
            {
                m_selectionStartIndex = m_cursorIndex;
            }
            else
            {
                m_cursorIndex = m_selectionStartIndex;
            }
            m_isCursorVisible = true;
            m_cursorTickCount = 0;
        }
        // If there is no selection and the cursor has space to go right
        else if (m_cursorIndex < m_displayText.getString().getSize())
        {
            // Modifier key
            if (m_inputManager.isModifierKeyHeld())
            {
                moveCursorToNextSpace();
            }
            else
            {
                moveCursorRight();
            }
            if (!m_inputManager.isShiftKeyHeld())
            {
                m_selectionStartIndex = m_cursorIndex;
            }
        }
    }
}

// Control the cursor with the mouse
void TextBox::controlCursorMouse()
{
    m_cursorTickCount = 0;
    m_isCursorVisible = true;

    sf::Vector2f mousePosition = static_cast<sf::Vector2f>(m_inputManager.getWindowMousePosition());
    sf::Vector2f cursorPosition = m_cursor.getPosition();
    if (mousePosition.x >= m_displayText.getPosition().x + m_displayText.getLocalBounds().width)
    {
        m_cursorIndex = static_cast<unsigned int>(m_displayText.getString().getSize());
        return;
    }
    else if (mousePosition.x <= m_displayText.getPosition().x)
    {
        m_cursorIndex = 0;
        return;
    }

    if (mousePosition.x > cursorPosition.x)
    {
        while ((m_displayText.findCharacterPos(m_cursorIndex).x + m_displayText.findCharacterPos(m_cursorIndex + 1).x) / 2 <
               mousePosition.x)
        {
            if (m_cursorIndex >= m_displayText.getString().getSize())
            {
                break;
            }
            m_cursorIndex++;
        }
    }
    else if (mousePosition.x < cursorPosition.x)
    {
        while ((m_displayText.findCharacterPos(m_cursorIndex).x + m_displayText.findCharacterPos(m_cursorIndex - 1).x) / 2 >=
               mousePosition.x)
        {
            m_cursorIndex--;
            if (m_cursorIndex <= 0)
            {
                break;
            }
        }
    }
}

void TextBox::dragCursor()
{
    float mouseDistance = 0;
    if (m_inputManager.getWindowMousePosition().x <= m_box.getPosition().x)
    {
        if (m_cursorIndex > 0)
        {
            mouseDistance = m_inputManager.getWindowMousePosition().x - m_box.getPosition().x;
        }
    }
    else if (m_cursorIndex < m_displayText.getString().getSize())
    {
        mouseDistance = m_inputManager.getWindowMousePosition().x - (m_box.getSize().x + m_box.getPosition().x);
    }

    m_dragCursorProgress += mouseDistance / m_dragCursorSpeedDivider;
    bool cursorMoved = false;

    while (m_dragCursorProgress <= -1 && m_cursorIndex > 0)
    {
        m_dragCursorProgress++;
        moveCursorLeft();
        cursorMoved = true;
    }
    while (m_dragCursorProgress >= 1 && m_cursorIndex < m_displayText.getString().getSize())
    {
        m_dragCursorProgress--;
        moveCursorRight();
        cursorMoved = true;
    }

    if (cursorMoved == true)
    {
        setTextPosition();
        drawTexture();
        setCursorPosition();
        setSelectionBounds();
    }
}

void TextBox::updateText()
{
    if (m_inputManager.detectedTextEnteredEvent() || m_inputManager.isKeyDescending(sf::Keyboard::Delete, true))
    {
        m_isCursorVisible = true;
        m_cursorTickCount = 0;
        for (const auto& enteredChar : m_inputManager.getEnteredText())
        {
            // If the text entered is not an undesirable character
            bool isValidAlphaNum = isCharacterAccepted(enteredChar) || // Valid char
                                   enteredChar == 3 || // CTRL + C
                                   enteredChar == 22 || // CTRL + V
                                   enteredChar == 24; // CTRL + X
            bool isValidNum = enteredChar >= 48 && enteredChar <= 57;
            if ((m_isDigitsOnly == false && isValidAlphaNum) || (m_isDigitsOnly == true && isValidNum))
            {
                if (enteredChar == 3 || enteredChar == 24) // CTRL + C or CTRL + X
                {
                    // If there is a selection
                    if (m_selectionStartIndex != m_cursorIndex)
                    {
                        sf::String selectedText;
                        if (m_selectionStartIndex < m_cursorIndex)
                        {
                            selectedText = m_text.substring(m_selectionStartIndex, m_cursorIndex - m_selectionStartIndex);
                        }
                        else
                        {
                            selectedText = m_text.substring(m_cursorIndex, m_selectionStartIndex - m_cursorIndex);
                        }
                        m_inputManager.setClipboardText(selectedText);
                        if (enteredChar == 24)
                        {
                            deleteSelection();
                            if (m_cursorIndex > m_selectionStartIndex)
                            {
                                m_cursorIndex = m_selectionStartIndex;
                            }
                            else
                            {
                                m_selectionStartIndex = m_cursorIndex;
                            }

                            setSelectionBounds();
                        }
                    }
                }
                else if (enteredChar == 22) // CTRL + V
                {
                    // Add all clipboard characters
                    for (const auto& clipboardChar : m_inputManager.getClipboardText())
                    {
                        sf::String pastedText;
                        if (isCharacterAccepted(clipboardChar) || clipboardChar == '\n')
                        {
                            if (clipboardChar == '\n')
                            {
                                pastedText += ' ';
                            }
                            else
                            {
                                pastedText += clipboardChar;
                            }
                            addText(pastedText);
                            m_selectionStartIndex = m_cursorIndex;
                        }
                    }
                }
                else
                {
                    // If there is a selection
                    if (m_selectionStartIndex != m_cursorIndex && enteredChar != 3 && enteredChar != 24)
                    {
                        // Delete the selection before adding a character
                        deleteSelection();

                        if (m_cursorIndex > m_selectionStartIndex)
                        {
                            m_cursorIndex = m_selectionStartIndex;
                        }
                        else
                        {
                            m_selectionStartIndex = m_cursorIndex;
                        }

                        setSelectionBounds();
                    }
                    addText(enteredChar);
                    if (m_textCanExceedBox == false &&
                        m_displayText.getLocalBounds().left + m_displayText.getLocalBounds().width > m_box.getSize().x - m_padding.x * 2)
                    {
                        sf::String text = m_displayText.getString();
                        text.erase(m_cursorIndex - 1);
                        m_cursorIndex--;
                        setDisplayText(text);
                    }
                }
                setTextPosition();
                drawTexture();
                setCursorPosition();
                if (enteredChar != 3)
                {
                    m_selectionStartIndex = m_cursorIndex;
                }
            }
            // If the text is a delete key
            else if (enteredChar == '\b' || enteredChar == 127)
            {
                if (m_displayText.getString().getSize() > 0)
                {
                    if (m_inputManager.isModifierKeyHeld())
                    {
                        // If CTRL + backspace, delete whole section
                        if (enteredChar == '\b')
                        {
                            moveCursorToPreviousSpace();
                        }
                        else
                        {
                            moveCursorToNextSpace();
                        }
                    }
                    // If there is a selection
                    if (m_selectionStartIndex != m_cursorIndex)
                    {
                        deleteSelection();

                        if (m_cursorIndex > m_selectionStartIndex)
                        {
                            m_cursorIndex = m_selectionStartIndex;
                        }
                        else
                        {
                            m_selectionStartIndex = m_cursorIndex;
                        }

                        setSelectionBounds();
                    }
                    else
                    {
                        sf::String text = m_text;
                        if (enteredChar == '\b')
                        {
                            if (m_cursorIndex > 0)
                            {
                                if (m_inputManager.isShiftKeyHeld())
                                {
                                    m_cursorIndex = 0;
                                    deleteSelection();
                                }
                                else
                                {
                                    text.erase(m_cursorIndex - 1);
                                    m_cursorIndex--;
                                    setDisplayText(text);
                                }
                            }
                        }
                        else if (enteredChar == 127)
                        {
                            moveCursorToPreviousSpace();
                            deleteSelection();
                        }
                    }
                    setTextPosition();
                    drawTexture();
                    setCursorPosition();
                }
                m_selectionStartIndex = m_cursorIndex;
            }
            else if (enteredChar == 1) // CTRL + A
            {
                m_selectionStartIndex = 0;
                m_cursorIndex = static_cast<unsigned int>(m_displayText.getString().getSize());
                setTextPosition();
                drawTexture();
                setSelectionBounds();
                setCursorPosition();
            }
        }
        if (m_inputManager.isKeyDescending(sf::Keyboard::Delete, true))
        {
            if (m_cursorIndex < m_displayText.getString().getSize())
            {
                sf::String text = m_displayText.getString();
                if (m_inputManager.isModifierKeyHeld())
                {
                    moveCursorToNextSpace();
                    deleteSelection();
                    m_selectionStartIndex = m_cursorIndex;
                    setTextPosition();
                    setCursorPosition();
                }
                else
                {
                    text.erase(m_cursorIndex);
                    setDisplayText(text);
                    setTextPosition();
                }
                drawTexture();
            }
        }
    }
}

// Set the position of the text
void TextBox::setTextPosition()
{
    // If the text is bigger than the box
    if (m_displayText.getLocalBounds().left + m_displayText.getLocalBounds().width >= m_box.getSize().x - 2 * m_padding.x)
    {
        // If there is a gap between the right side of the box and the text, move the text
        if (m_displayText.getGlobalBounds().left + m_displayText.getLocalBounds().left + m_displayText.getLocalBounds().width <
            m_box.getPosition().x + m_box.getSize().x - m_padding.x)
        {
            m_displayText.move(m_box.getPosition().x + m_box.getSize().x - m_padding.x -
                                   (m_displayText.getGlobalBounds().left + m_displayText.getLocalBounds().left +
                                    m_displayText.getLocalBounds().width),
                               0);
        }

        // If the cursor's character is out of bounds, move the text
        if (m_displayText.findCharacterPos(m_cursorIndex).x - m_displayText.getLocalBounds().left >
            m_box.getPosition().x + m_box.getSize().x - m_padding.x)
        {
            m_displayText.move(m_box.getPosition().x + m_box.getSize().x - m_padding.x - m_displayText.findCharacterPos(m_cursorIndex).x +
                                   m_displayText.getLocalBounds().left,
                               0);
        }
        else if (m_displayText.findCharacterPos(m_cursorIndex).x < m_box.getPosition().x + m_padding.x)
        {
            m_displayText.move(m_box.getPosition().x + m_padding.x - m_displayText.findCharacterPos(m_cursorIndex).x, 0);
        }
    }
    // If the text is smaller than the box and is not on the left of the box
    else if (m_displayText.getPosition().x != m_box.getPosition().x + m_padding.x)
    {
        m_displayText.move(m_box.getPosition().x + m_padding.x - m_displayText.getPosition().x, 0);
    }
}

void TextBox::addText(const sf::String& enteredText)
{
    // Check if text is too big
    if (m_displayText.getString().getSize() < m_maxTextLength || m_maxTextLength == 0)
    {
        // Add the character where the cursor is
        sf::String text = m_text;
        if (m_cursorIndex != m_selectionStartIndex)
        {
            if (m_cursorIndex < m_selectionStartIndex)
            {
                text.erase(m_cursorIndex, m_selectionStartIndex - m_cursorIndex);
            }
            else
            {
                text.erase(m_selectionStartIndex, m_cursorIndex - m_selectionStartIndex);
                m_cursorIndex -= (m_cursorIndex - m_selectionStartIndex);
            }
        }
        text.insert(m_cursorIndex, enteredText);
        setDisplayText(text);
        m_cursorIndex++;
    }
}

void TextBox::deleteSelection()
{
    sf::String text = m_displayText.getString();
    if (m_cursorIndex < m_selectionStartIndex)
    {
        text.erase(m_cursorIndex, m_selectionStartIndex - m_cursorIndex);
    }
    else
    {
        text.erase(m_selectionStartIndex, m_cursorIndex - m_selectionStartIndex);
        m_cursorIndex -= (m_cursorIndex - m_selectionStartIndex);
    }
    setText(text);
}

void TextBox::resetSelection()
{
    m_selectionStartIndex = m_cursorIndex;
    if (m_selection.getSize().x > 0)
    {
        setSelectionBounds();
    }
}

// Set the position and size of the selection box
void TextBox::setSelectionBounds()
{
    sf::Vector2f position = sf::Vector2f(m_displayText.findCharacterPos(m_selectionStartIndex).x, m_selection.getPosition().y);
    sf::Vector2f dimensions =
        sf::Vector2f(m_displayText.findCharacterPos(m_cursorIndex).x - m_displayText.findCharacterPos(m_selectionStartIndex).x,
                     m_selection.getSize().y);
    if (dimensions.x < 0)
    {
        position.x += dimensions.x;
        dimensions.x = -dimensions.x;
    }

    if (position.x < m_box.getPosition().x)
    {
        dimensions.x -= (m_box.getPosition().x - position.x);
        position.x = m_box.getPosition().x;
    }

    if (position.x + dimensions.x > m_box.getPosition().x + m_box.getSize().x)
    {
        dimensions.x = m_position.x + m_box.getSize().x - position.x;
    }
    m_selection.setPosition(position);
    m_selection.setSize(dimensions);
}

void TextBox::moveCursorToPreviousSpace()
{
    while (m_cursorIndex > 0)
    {
        // Move cursor
        moveCursorLeft();
        // Until there is a space
        if (m_cursorIndex > 1 && m_displayText.getString()[m_cursorIndex - 1] == ' ')
        {
            break;
        }
    }
}

void TextBox::moveCursorToNextSpace()
{
    while (m_cursorIndex < m_displayText.getString().getSize())
    {
        // Move the cursor
        moveCursorRight();
        // Until there is a space
        if (m_cursorIndex < m_displayText.getString().getSize() && m_displayText.getString()[m_cursorIndex] == ' ')
        {
            break;
        }
    }
}

void TextBox::update()
{
    if (m_isReadOnly == false)
    {
        // Update cursor blinking
        m_cursorTickCount++;
        if (m_cursorTickCount >= m_cursorBlinkSpeed)
        {
            m_isCursorVisible = !m_isCursorVisible;
            m_cursorTickCount = 0;
        }
    }
}

void TextBox::handleInput()
{
    if (m_isReadOnly == false)
    {
        if (m_hasFocus == true)
        {
            updateText();
            updateCursor();
        }

        // Check if there is a mouse press
        if (m_inputManager.isMouseButtonDescending(sf::Mouse::Left))
        {
            bool isMouseInsideBox = checkMousePosition();
            if (isMouseInsideBox == true && m_hasFocus == false)
            {
                m_hasFocus = true;
                controlCursorMouse();
                setCursorPosition();
                m_box.setOutlineColor(m_outlineColorFocused);
                resetSelection();
            }
            else if (isMouseInsideBox == false && m_hasFocus == true)
            {
                m_hasFocus = false;
                m_box.setOutlineColor(m_outlineColor);
                resetSelection();
            }
        }

        // Check if the window lost focus
        if (m_inputManager.detectedLostFocusEvent())
        {
            m_hasFocus = false;
            m_box.setOutlineColor(m_outlineColor);
            resetSelection();
        }
    }
}

void TextBox::setPosition(const sf::Vector2f& position)
{
    m_position = position;
    m_box.setPosition(position);
    centerText();
    m_cursor.setPosition(position.x + m_padding.x, m_displayText.getPosition().y);
    m_selection.setPosition({position.x, m_cursor.getPosition().y});
    m_renderTexture.setView(sf::View(sf::FloatRect(position.x, position.y, m_box.getSize().x, m_box.getSize().y)));
    m_renderSprite.setPosition(position);
    setPaddingSize(m_padding);
}

void TextBox::setDimensions(const sf::Vector2f& dimensions)
{
    if (dimensions.x == 0 || dimensions.y == 0)
    {
        return;
    }

    m_box.setSize(dimensions);
    centerText();
    m_cursor.setPosition(m_position.x + m_padding.x, m_displayText.getPosition().y);
    m_selection.setPosition({m_box.getPosition().x, m_cursor.getPosition().y});
    setPaddingSize(m_padding);
    m_renderTexture.create(dimensions.x, dimensions.y);
    m_renderTexture.setView(sf::View(sf::FloatRect(m_box.getPosition().x, m_box.getPosition().y, dimensions.x, dimensions.y)));
    m_renderSprite.setTexture(m_renderTexture.getTexture(), true);
    setPaddingSize(m_padding);
    drawTexture();
}

void TextBox::setPaddingSize(const sf::Vector2i& padding)
{
    m_padding = padding;
    m_hideBoxLeft[0].position = m_position;
    m_hideBoxLeft[1].position = m_position + sf::Vector2f(0, m_box.getSize().y);
    m_hideBoxLeft[2].position = m_position + sf::Vector2f(m_padding.x * m_opaquePaddingProportion, 0);
    m_hideBoxLeft[3].position = m_position + sf::Vector2f(m_padding.x * m_opaquePaddingProportion, m_box.getSize().y);
    m_hideBoxLeft[4].position = m_position + sf::Vector2f(m_padding.x, 0);
    m_hideBoxLeft[5].position = m_position + sf::Vector2f(m_padding.x, m_box.getSize().y);
    m_hideBoxRight[0].position = m_position + sf::Vector2f(m_box.getSize().x, 0);
    m_hideBoxRight[1].position = m_position + m_box.getSize();
    m_hideBoxRight[2].position = m_position + sf::Vector2f(m_box.getSize().x - (m_padding.x * m_opaquePaddingProportion), 0);
    m_hideBoxRight[3].position =
        m_position + sf::Vector2f(m_box.getSize().x - (m_padding.x * m_opaquePaddingProportion), m_box.getSize().y);
    m_hideBoxRight[4].position = m_position + sf::Vector2f(m_box.getSize().x - (m_padding.x), 0);
    m_hideBoxRight[5].position = m_position + sf::Vector2f(m_box.getSize().x - (m_padding.x), m_box.getSize().y);
}

void TextBox::setCharacterSize(int characterSize)
{
    m_displayText.setCharacterSize(characterSize);
    m_backgroundText.setCharacterSize(characterSize);
    m_cursor.setSize(sf::Vector2f(m_cursor.getSize().x, m_displayText.getFont()->getLineSpacing(m_displayText.getCharacterSize())));
    m_selection.setSize({m_selection.getSize().x, m_cursor.getSize().y});
    centerText();
}

void TextBox::setBackgroundColor(sf::Color backgroundColor)
{
    m_box.setFillColor(backgroundColor);
    sf::Color boxColorFade = backgroundColor;
    boxColorFade.a = m_startingOpacity;
    backgroundColor.a = m_endingOpacity;
    m_hideBoxLeft[0].color = backgroundColor;
    m_hideBoxLeft[1].color = backgroundColor;
    m_hideBoxLeft[2].color = backgroundColor;
    m_hideBoxLeft[3].color = backgroundColor;
    m_hideBoxLeft[4].color = boxColorFade;
    m_hideBoxLeft[5].color = boxColorFade;
    m_hideBoxRight[0].color = backgroundColor;
    m_hideBoxRight[1].color = backgroundColor;
    m_hideBoxRight[2].color = backgroundColor;
    m_hideBoxRight[3].color = backgroundColor;
    m_hideBoxRight[4].color = boxColorFade;
    m_hideBoxRight[5].color = boxColorFade;
}

void TextBox::setFont(const sf::Font& font)
{
    m_displayText.setFont(font);
    m_backgroundText.setFont(font);
    centerText();
}

void TextBox::setText(const sf::String& text)
{
    setDisplayText(text);
    setTextPosition();
    m_selectionStartIndex = m_cursorIndex;
    setCursorPosition();
    drawTexture();
}

void TextBox::setFocus(bool hasFocus)
{
    if (m_hasFocus != hasFocus)
    {
        m_hasFocus = hasFocus;
        if (m_hasFocus == true)
        {
            m_box.setOutlineColor(m_outlineColorFocused);
        }
        else
        {
            m_box.setOutlineColor(m_outlineColor);
        }
    }
}

void TextBox::setPasswordModeEnabled(bool isPasswordModeEnabled)
{
    m_isPasswordModeEnabled = isPasswordModeEnabled;
    setDisplayText(m_text);
}
