#ifndef FEHKEYBOARD_H
#define FEHKEYBOARD_H

#include <stdint.h>
#include <cstring>
#include <cstdint>
#include <string>
#include "tigr.h"
#include <FEHLCD.h>
#include <vector>


#define ALPHABET_KEYSET KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z
#define WASD_KEYSET KEY_W, KEY_A, KEY_S, KEY_D
#define ARROW_KEYSET KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT



#ifdef _WIN32
#include <windows.h>
enum Key
{
    KEY_NONE = 0,

    KEY_A = 'A',
    KEY_B = 'B',
    KEY_C = 'C',
    KEY_D = 'D',
    KEY_E = 'E',
    KEY_F = 'F',
    KEY_G = 'G',
    KEY_H = 'H',
    KEY_I = 'I',
    KEY_J = 'J',
    KEY_K = 'K',
    KEY_L = 'L',
    KEY_M = 'M',
    KEY_N = 'N', 
    KEY_O = 'O',
    KEY_P = 'P',
    KEY_Q = 'Q',
    KEY_R = 'R',
    KEY_S = 'S',
    KEY_T = 'T',
    KEY_U = 'U',
    KEY_V = 'V',
    KEY_W = 'W',
    KEY_X = 'X',
    KEY_Y = 'Y',
    KEY_Z = 'Z', 

    KEY_SHIFT = VK_SHIFT,
    KEY_CONTROL = VK_CONTROL,
    KEY_ALT = VK_MENU,
    KEY_SPACE = VK_SPACE,
    KEY_ENTER = VK_RETURN,
    KEY_BACKSPACE = VK_BACK,
    KEY_TAB = VK_TAB,
    KEY_ESCAPE = VK_ESCAPE,
    
    KEY_UP = VK_UP,
    KEY_DOWN = VK_DOWN,
    KEY_LEFT = VK_LEFT,
    KEY_RIGHT = VK_RIGHT,
};
#else
enum Key
{
    KEY_NONE = 0,

    KEY_A = 'A',
    KEY_B = 'B',
    KEY_C = 'C',
    KEY_D = 'D',
    KEY_E = 'E',
    KEY_F = 'F',
    KEY_G = 'G',
    KEY_H = 'H',
    KEY_I = 'I',
    KEY_J = 'J',
    KEY_K = 'K',
    KEY_L = 'L',
    KEY_M = 'M',
    KEY_N = 'N', 
    KEY_O = 'O',
    KEY_P = 'P',
    KEY_Q = 'Q',
    KEY_R = 'R',
    KEY_S = 'S',
    KEY_T = 'T',
    KEY_U = 'U',
    KEY_V = 'V',
    KEY_W = 'W',
    KEY_X = 'X',
    KEY_Y = 'Y',
    KEY_Z = 'Z', 

    KEY_SHIFT = TKey::TK_LSHIFT,
    KEY_CONTROL = TKey::TK_LCONTROL,
    KEY_ALT = TKey::TK_LALT,
    KEY_SPACE = TKey::TK_SPACE,
    KEY_ENTER = TKey::TK_RETURN,
    KEY_BACKSPACE = TKey::TK_BACKSPACE,
    KEY_TAB = TKey::TK_TAB,
    KEY_ESCAPE = TKey::TK_ESCAPE,
    
    KEY_UP = TKey::TK_UP,
    KEY_DOWN = TKey::TK_DOWN,
    KEY_LEFT = TKey::TK_LEFT,
    KEY_RIGHT = TKey::TK_RIGHT,
};
#endif


class FEHKeyboard
{

public:
    FEHKeyboard() {}

    /// @brief Return the last character pressed
    /// @note This function will interpret the key combination pressed. e.g. Shift + A = 'A' whereas A = 'a'
    /// @return 
    char lastChar();

    /// @brief Check if a specific key is pressed
    /// @param key Key to check
    /// @return Boolean value indicating if the key is pressed
    bool isPressed(Key key);

    /// @brief Check if a specific set of keys are pressed
    /// @param keys Keys to be pressed simultaneously. e.g. {KEY_A, KEY_B}
    /// @note All keys must be pressed to return true
    /// @return Boolean value indicating if all keys are pressed
    bool isPressed(const std::vector<Key>& keys);

    /// @brief Check if any keys are pressed
    /// @param keys Specific keys to check. If not provided, all keys are checked
    /// @return Boolean value indicating if any specified keys are pressed
    bool areAnyPressed(const std::vector<Key>& keys = {});

    /// @brief Wait for a key to be pressed
    /// @param timeout Time to wait for a key to be pressed, in seconds
    /// @note If timeout is 0, the function will wait indefinitely
    /// @note If timeout is negative, the function will return immediately
    /// @note If function times out, false is returned
    /// @return If a key was pressed
    bool waitAnyKey(double timeout = 0);

    /// @brief Wait for any of the specified keys to be pressed
    /// @param keys Keys that are valid to be pressed
    /// @param timeout Time to wait for a key to be pressed, in seconds
    /// @note If timeout is 0, the function will wait indefinitely
    /// @note If timeout is negative, the function will return immediately
    /// @note If function times out, false is returned
    /// @return If a key in the set was pressed
    bool waitAnyOfKeys(const std::vector<Key>& keys, double timeout = 0);

    /// @brief Wait for a specific key to be pressed
    /// @param key Key to wait for
    /// @param timeout Time to wait for the key to be pressed, in seconds
    /// @note If timeout is 0, the function will wait indefinitely
    /// @note If timeout is negative, the function will return immediately
    /// @return Boolean value indicating if the key was pressed
    bool waitSpecificKey(Key key, double timeout = 0);

    /// @brief Wait for a specific set of keys to be pressed
    /// @param keys Keys to be pressed simultaneously. e.g. {KEY_A, KEY_B}
    /// @param timeout Time to wait for the keys to be pressed, in seconds
    /// @note If timeout is 0, the function will wait indefinitely
    /// @note If timeout is negative, the function will return immediately
    /// @return Boolean value indicating if all keys were pressed
    bool waitSpecificKeys(const std::vector<Key>& keys, double timeout = 0);

    /// @brief Wait for no keys to be pressed
    /// @param timeout Time to wait for no keys to be pressed, in seconds
    /// @note If timeout is 0, the function will wait indefinitely
    /// @note If timeout is negative, the function will return immediately
    /// @return Boolean value indicating if no keys were pressed
    bool waitNoKeys(double timeout = 0);

    /// @brief Wait for no keys to be pressed except for the specified keys
    /// @param keys Keys that are valid to be pressed
    /// @param timeout Time to wait for no keys to be pressed, in seconds
    /// @note If timeout is 0, the function will wait indefinitely
    /// @note If timeout is negative, the function will return immediately
    /// @return Boolean value indicating if no keys were pressed except for the specified keys
    bool waitNoKeysExcept(const std::vector<Key>& keys, double timeout = 0);

    /// @brief Return a list of currently pressed scan codes (0..255)
    std::vector<int> pressedScanCodes();

    /// @brief Return a list of currently pressed keys mapped to the Key enum where possible
    std::vector<Key> pressedKeys();



    private:

    /// @brief Makes masks for each key set
    /// @param keys Keys of interest
    /// @return vector of 8 uint32_t masks
    std::vector<uint32_t> makeMasks(const std::vector<Key>& keys = {});

    void debugPrint();

    /// @brief Check if any keys are pressed
    /// @param masks Predefined masks to check
    /// @return Boolean value indicating if any keys in the mask are pressed
    bool areAnyPressed(std::vector<uint32_t>& masks);
};

extern FEHKeyboard Keyboard;

#endif // FEHKEYBOARD_H
