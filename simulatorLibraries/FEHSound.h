#ifndef FEHSOUND_H
#define FEHSOUND_H

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

// Forward declarations for platform-specific types
#ifdef __APPLE__
  #ifdef __OBJC__
    #import <Cocoa/Cocoa.h>
  #else
    class NSSound;  // forward-declare for pure C++ compile
  #endif
#endif

class FEHSound
{
public:
    // Constructor takes the file path to a .wav file.
    FEHSound(const std::string &filepath);
    ~FEHSound();

    // Playback control methods (cross-platform interface)
    void play();
    void pause();
    void restart();
    void playFrom(double time); // time in seconds
    
    // Volume control methods
    void setVolume(double vol); // Set volume in range [0.0, 1.0]
    double getVolume() const;   // Get current volume setting

private:
    // Common members (all platforms)
    std::string filepath;
    double volume;
    
    // WAV file data for software volume control
    std::vector<char> wavData;
    bool wavLoaded;
    std::string tempFilePath;
    double lastScaledVolume;
    bool tempFileValid;
    
    // Common helper methods
    bool loadWavFile();
    bool createScaledWavFile();
    void applyVolumeToSamples(std::vector<char>& data, int sampleSize, int numChannels, double vol);

    // Platform-specific implementation methods
    void platformInit();
    void platformCleanup();
    void platformPlay();
    void platformPause();
    void platformRestart();
    void platformPlayFrom(double time);
    void platformOpenScaledFile();

    // Platform-specific members
#ifdef _WIN32
    std::string m_alias; // Unique alias for MCI commands
#elif defined(__APPLE__)
    NSSound *nsSound;    // Cocoa sound object
    double pausedTime;   // Track the current playback time when paused
#elif defined(__linux__)
    // TODO: Linux-specific implementation members
#endif
};

#endif // FEHSOUND_H
