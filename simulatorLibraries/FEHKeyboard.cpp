#include "FEHKeyboard.h"
// #include "tigr.h"
// #include "FEHLCD.h"
#include "FEHUtility.h"


FEHKeyboard Keyboard;


char FEHKeyboard::lastChar()
{
    return tigrReadChar(LCD.getScreen());
}

bool FEHKeyboard::isPressed(Key key)
{
    // Treat modifiers as either left or right; avoid signed-char truncation.
    Tigr* scr = LCD.getScreen();
    switch (key) {
        case KEY_SHIFT:
            return tigrKeyHeld(scr, (int)TK_LSHIFT) || tigrKeyHeld(scr, (int)TK_RSHIFT) || tigrKeyHeld(scr, (int)TK_SHIFT);
        case KEY_CONTROL:
            return tigrKeyHeld(scr, (int)TK_LCONTROL) || tigrKeyHeld(scr, (int)TK_RCONTROL) || tigrKeyHeld(scr, (int)TK_CONTROL);
        case KEY_ALT:
            return tigrKeyHeld(scr, (int)TK_LALT) || tigrKeyHeld(scr, (int)TK_RALT) || tigrKeyHeld(scr, (int)TK_ALT);
        default:
            return tigrKeyHeld(scr, (int)key);
    }
}


bool FEHKeyboard::isPressed(const std::vector<Key>& keys)
{
    debugPrint();
    for (Key key : keys)
    {
        printf("Key: %d\n", key);
    if (!tigrKeyHeld(LCD.getScreen(), (char)key))
        {
            return false;
        }
    }
    return true;
}

bool FEHKeyboard::areAnyPressed(const std::vector<Key>& keys)
{
    // Make AND-masks for each key set
    std::vector<uint32_t> masks = makeMasks(keys);

    // Get entire keystates
    std::vector<uint32_t> data = tigrKeyboardData(LCD.getScreen());

    // Check if any keys are pressed
    for (int i = 0; i < 8; i++)
    {
        if (data[i] & masks[i])
        {
            return true;
        }
    }

    return false;
}

bool FEHKeyboard::areAnyPressed(std::vector<uint32_t>& masks)
{
    // Get entire keystates
    std::vector<uint32_t> data = tigrKeyboardData(LCD.getScreen());

    // Check if any keys are pressed
    for (int i = 0; i < 8; i++)
    {
        if (data[i] & masks[i])
        {
            return true;
        }
    }

    return false;

}


bool FEHKeyboard::waitAnyKey(double timeout)
{
    double startTime = TimeNow();

    if (timeout < 0)
        return false;

    while (timeout == 0 || TimeNow() - startTime < timeout)
    {
        if (areAnyPressed())
        {
            return true;
        }
    tigrUpdate(LCD.getScreen());
    }

    return false;
}


bool FEHKeyboard::waitAnyOfKeys(const std::vector<Key>& keys, double timeout)
{
    double startTime = TimeNow();

    if (timeout < 0)
        return false;

    std::vector<uint32_t> masks = makeMasks(keys);

    while (timeout == 0 || TimeNow() - startTime < timeout)
    {

        if (areAnyPressed(masks))
        {
            return true;
        }

    tigrUpdate(LCD.getScreen());
    }

    return false;
}



bool FEHKeyboard::waitSpecificKey(Key key, double timeout)
{
    double startTime = TimeNow();

    if (timeout < 0)
        return false;


    while (timeout == 0 || TimeNow() - startTime < timeout)
    {
        if (isPressed(key))
        {
            return true;
        }

        tigrUpdate(LCD.getScreen());
    }

    return false;
}


bool FEHKeyboard::waitSpecificKeys(const std::vector<Key>& keys, double timeout)
{
    double startTime = TimeNow();

    if (timeout < 0)
        return false;

    while (timeout == 0 || TimeNow() - startTime < timeout)
    {

        if (isPressed(keys))
        {
            return true;
        }

    tigrUpdate(LCD.getScreen());
    }

    return false;
}





bool FEHKeyboard::waitNoKeys(double timeout)
{
    double startTime = TimeNow();

    if (timeout < 0)
        return false;


    std::vector<uint32_t> masks = makeMasks();

    while (timeout == 0 || TimeNow() - startTime < timeout)
    {

        if (!areAnyPressed(masks))
        {
            return true;
        }

    tigrUpdate(LCD.getScreen());
    }

    return false;
}


bool FEHKeyboard::waitNoKeysExcept(const std::vector<Key>& keys, double timeout)
{
    double startTime = TimeNow();

    if (timeout < 0)
        return false;

    std::vector<uint32_t> masks = makeMasks(keys);

    // Invert masks
    for (int i = 0; i < 8; i++)
    {
        masks[i] = ~masks[i];
    }

    while (timeout == 0 || TimeNow() - startTime < timeout)
    {

        if (!areAnyPressed(masks))
        {
            return true;
        }

    tigrUpdate(LCD.getScreen());
    }

    return false;
}




std::vector<uint32_t> FEHKeyboard::makeMasks(const std::vector<Key>& keys)
{
    // Make AND-masks for each key set
    std::vector<uint32_t> masks({0, 0, 0, 0, 0, 0, 0, 0});

    if (keys.empty())
    {
        for (int i = 0; i < 8; i++)
        {
            masks[i] = 0xFFFFFFFF;
        }
    }
    else{
        for (Key key : keys)
        {
            auto addCode = [&masks](int code){
                if (code >= 0 && code < 256) {
                    int block = code / 32;
                    int bit = code % 32;
                    masks[block] |= (1u << bit);
                }
            };

            switch (key) {
                case KEY_SHIFT:
                    addCode((int)TK_SHIFT);    // some platforms set TK_SHIFT
                    addCode((int)TK_LSHIFT);
                    addCode((int)TK_RSHIFT);
                    break;
                case KEY_CONTROL:
                    addCode((int)TK_CONTROL);
                    addCode((int)TK_LCONTROL);
                    addCode((int)TK_RCONTROL);
                    break;
                case KEY_ALT:
                    addCode((int)TK_ALT);
                    addCode((int)TK_LALT);
                    addCode((int)TK_RALT);
                    break;
                default:
                    addCode((int)key);
                    break;
            }
        }
    }

    return masks;
}



void FEHKeyboard::debugPrint()
{
    std::vector<uint32_t> data = tigrKeyboardData(LCD.getScreen());

    printf("Active keys: ");

    // Print out scan codes of active keys

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (data[i] & (1 << j))
            {
                printf("%d ", i * 32 + j);
            }
        }
    }

    printf("\n");



}


std::vector<int> FEHKeyboard::pressedScanCodes()
{
    std::vector<int> pressed;
    std::vector<uint32_t> data = tigrKeyboardData(LCD.getScreen());

    for (int block = 0; block < (int)data.size(); block++) {
        uint32_t mask = data[block];
        for (int bit = 0; bit < 32; bit++) {
            if (mask & (1u << bit)) {
                pressed.push_back(block * 32 + bit);
            }
        }
    }

    return pressed;
}

std::vector<Key> FEHKeyboard::pressedKeys()
{
    std::vector<Key> keys;
    std::vector<int> codes = pressedScanCodes();
    for (int c : codes) {
        if (c >= 0 && c < 256) {
            keys.push_back((Key)c);
        }
    }
    return keys;
}