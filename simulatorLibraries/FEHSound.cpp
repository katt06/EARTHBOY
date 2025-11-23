#include "FEHSound.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <cstdint>

#if !defined(_WIN32)
#include <unistd.h>
#endif

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <sstream>
static int soundCounter = 0; // Counter for unique MCI aliases
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include <objc/message.h>
#endif

// =============================================================================
// COMMON CROSS-PLATFORM IMPLEMENTATION
// =============================================================================

FEHSound::FEHSound(const std::string &filepath) : filepath(filepath)
{
    // Initialize common members
    volume = 1.0;
    wavLoaded = false;
    tempFilePath = "";
    lastScaledVolume = -1.0;
    tempFileValid = false;
    
    // Load WAV data for software volume control
    loadWavFile();
    
    // Platform-specific initialization
    platformInit();
}

FEHSound::~FEHSound()
{
    // Platform-specific cleanup
    platformCleanup();
    
    // Clean up temporary files
    if (!tempFilePath.empty())
    {
        std::remove(tempFilePath.c_str());
    }
}

void FEHSound::play()
{
    if (!createScaledWavFile())
    {
        std::cerr << "Error: Could not create volume-scaled audio file" << std::endl;
        return;
    }
    platformPlay();
}

void FEHSound::pause()
{
    platformPause();
}

void FEHSound::restart()
{
    if (!createScaledWavFile())
    {
        std::cerr << "Error: Could not create volume-scaled audio file" << std::endl;
        return;
    }
    platformRestart();
}

void FEHSound::playFrom(double time)
{
    if (!createScaledWavFile())
    {
        std::cerr << "Error: Could not create volume-scaled audio file" << std::endl;
        return;
    }
    platformPlayFrom(time);
}

void FEHSound::setVolume(double vol)
{
    // Clamp to [0.0, 1.0]
    if (vol < 0.0) vol = 0.0;
    else if (vol > 1.0) vol = 1.0;

    volume = vol;

    // Invalidate temp file if volume changed
    if (vol != lastScaledVolume)
    {
        tempFileValid = false;
    }
}

double FEHSound::getVolume() const
{
    return volume;
}

bool FEHSound::loadWavFile()
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open WAV file: " << filepath << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    wavData.resize(fileSize);
    file.read(wavData.data(), fileSize);
    file.close();

    std::cout << "Loaded WAV file: " << filepath << " (" << fileSize << " bytes)" << std::endl;
    wavLoaded = true;
    return true;
}

bool FEHSound::createScaledWavFile()
{
    if (!wavLoaded || wavData.empty())
    {
        return false;
    }

    // If temp file is valid and volume hasn't changed, reuse it
    if (tempFileValid && volume == lastScaledVolume && !tempFilePath.empty())
    {
        return true;
    }

    // Make a copy of the WAV data to modify
    std::vector<char> scaledData = wavData;

    // Simple WAV format parsing
    if (scaledData.size() < 44)
    {
        std::cerr << "Error: WAV file too small" << std::endl;
        return false;
    }

    // Check for RIFF and WAVE headers
    if (std::memcmp(scaledData.data(), "RIFF", 4) != 0 ||
        std::memcmp(scaledData.data() + 8, "WAVE", 4) != 0)
    {
        std::cerr << "Error: Invalid WAV file format" << std::endl;
        return false;
    }

    // Find the "data" chunk
    size_t dataOffset = 0;
    size_t pos = 12;
    while (pos + 8 < scaledData.size())
    {
        if (std::memcmp(scaledData.data() + pos, "data", 4) == 0)
        {
            dataOffset = pos + 8;
            break;
        }
        uint32_t chunkSize = *reinterpret_cast<uint32_t*>(scaledData.data() + pos + 4);
        pos += 8 + chunkSize;
    }

    if (dataOffset == 0)
    {
        std::cerr << "Error: Could not find data chunk in WAV file" << std::endl;
        return false;
    }

    // Get format info from fmt chunk
    int numChannels = 2;
    int bitsPerSample = 16;
    
    pos = 12;
    while (pos + 8 < dataOffset)
    {
        if (std::memcmp(scaledData.data() + pos, "fmt ", 4) == 0)
        {
            numChannels = *reinterpret_cast<uint16_t*>(scaledData.data() + pos + 10);
            bitsPerSample = *reinterpret_cast<uint16_t*>(scaledData.data() + pos + 22);
            break;
        }
        uint32_t chunkSize = *reinterpret_cast<uint32_t*>(scaledData.data() + pos + 4);
        pos += 8 + chunkSize;
    }

    // Apply volume scaling to audio samples
    applyVolumeToSamples(scaledData, bitsPerSample / 8, numChannels, volume);

    // Generate unique temp filename
    static int tempCounter = 0;
    std::string newTempPath;
#ifdef _WIN32
    char tempDir[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempDir) == 0)
    {
        newTempPath = "fehsound_" + std::to_string(GetCurrentProcessId()) + "_" + std::to_string(tempCounter++) + ".wav";
    }
    else
    {
        newTempPath = std::string(tempDir) + "fehsound_" + std::to_string(GetCurrentProcessId()) + "_" + std::to_string(tempCounter++) + ".wav";
    }
#else
    newTempPath = "/tmp/fehsound_" + std::to_string(getpid()) + "_" + std::to_string(tempCounter++) + ".wav";
#endif

    // Write scaled data to temporary file
    std::ofstream outFile(newTempPath, std::ios::binary | std::ios::trunc);
    if (!outFile)
    {
        std::cerr << "Error: Could not create temporary WAV file: " << newTempPath << std::endl;
        return false;
    }

    outFile.write(scaledData.data(), scaledData.size());
    outFile.close();

    std::cout << "Created scaled WAV file: " << newTempPath << " with volume " << volume << std::endl;

    // If we already had a temp file, remove it now that we've created a new one.
    std::string previousTemp = tempFilePath;

    // Update to the new temp file
    tempFilePath = newTempPath;
    lastScaledVolume = volume;
    tempFileValid = true;

    if (!previousTemp.empty() && previousTemp != tempFilePath)
    {
        if (std::remove(previousTemp.c_str()) != 0)
        {
            // Non-fatal: log but continue. This avoids leaking many temp files.
            std::cerr << "Warning: could not remove previous temporary file: " << previousTemp << std::endl;
        }
    }

    return true;
}

void FEHSound::applyVolumeToSamples(std::vector<char>& data, int sampleSize, int numChannels, double vol)
{
    // Find data chunk offset
    size_t dataOffset = 0;
    size_t pos = 12;
    while (pos + 8 < data.size())
    {
        if (std::memcmp(data.data() + pos, "data", 4) == 0)
        {
            dataOffset = pos + 8;
            break;
        }
        uint32_t chunkSize = *reinterpret_cast<uint32_t*>(data.data() + pos + 4);
        pos += 8 + chunkSize;
    }

    if (dataOffset == 0) return;

    uint32_t dataSize = *reinterpret_cast<uint32_t*>(data.data() + dataOffset - 4);
    
    // Apply minimum volume to avoid completely silent files
    double effectiveVol = vol;
    if (effectiveVol < 0.001) effectiveVol = 0.001;
    
    if (sampleSize == 2) // 16-bit samples
    {
        int16_t* samples = reinterpret_cast<int16_t*>(data.data() + dataOffset);
        size_t numSamples = dataSize / 2;
        
        for (size_t i = 0; i < numSamples; ++i)
        {
            double scaledSample = samples[i] * effectiveVol;
            scaledSample = std::max(-32768.0, std::min(32767.0, scaledSample));
            samples[i] = static_cast<int16_t>(scaledSample);
        }
    }
    else if (sampleSize == 1) // 8-bit samples
    {
        uint8_t* samples = reinterpret_cast<uint8_t*>(data.data() + dataOffset);
        
        for (size_t i = 0; i < dataSize; ++i)
        {
            double scaledSample = (samples[i] - 128) * effectiveVol + 128;
            scaledSample = std::max(0.0, std::min(255.0, scaledSample));
            samples[i] = static_cast<uint8_t>(scaledSample);
        }
    }
}

// =============================================================================
// WINDOWS IMPLEMENTATION
// =============================================================================
#ifdef _WIN32

void FEHSound::platformInit()
{
    // Generate a unique alias name
    std::stringstream ss;
    ss << "FEHSound_" << soundCounter++;
    m_alias = ss.str();

    // Open the original WAV file with MCI
    std::string command = "open \"" + filepath + "\" type waveaudio alias " + m_alias;
    MCIERROR error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        std::cerr << "Error opening sound file: " << filepath << std::endl;
    }
}

void FEHSound::platformCleanup()
{
    std::string command = "close " + m_alias;
    mciSendStringA(command.c_str(), nullptr, 0, nullptr);
}

void FEHSound::platformPlay()
{
    platformOpenScaledFile();
    
    std::string command = "play " + m_alias;
    MCIERROR error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        char errorBuffer[256];
        mciGetErrorStringA(error, errorBuffer, sizeof(errorBuffer));
        std::cerr << "Error playing sound: " << tempFilePath << " - MCI Error: " << errorBuffer << std::endl;
    }
}

void FEHSound::platformPause()
{
    std::string command = "pause " + m_alias;
    MCIERROR error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        std::cerr << "Error pausing sound: " << filepath << std::endl;
    }
}

void FEHSound::platformRestart()
{
    platformOpenScaledFile();
    
    std::string command = "play " + m_alias;
    MCIERROR error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        char errorBuffer[256];
        mciGetErrorStringA(error, errorBuffer, sizeof(errorBuffer));
        std::cerr << "Error playing sound: " << tempFilePath << " - MCI Error: " << errorBuffer << std::endl;
    }
}

void FEHSound::platformPlayFrom(double time)
{
    platformOpenScaledFile();
    
    // Convert time in seconds to milliseconds
    int ms = static_cast<int>(time * 1000);
    
    // Seek to the specified position
    std::string command = "seek " + m_alias + " to " + std::to_string(ms);
    MCIERROR error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        char errorBuffer[256];
        mciGetErrorStringA(error, errorBuffer, sizeof(errorBuffer));
        std::cerr << "Error seeking to " << time << " seconds: " << errorBuffer << std::endl;
        return;
    }

    // Play from that position
    command = "play " + m_alias;
    error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        char errorBuffer[256];
        mciGetErrorStringA(error, errorBuffer, sizeof(errorBuffer));
        std::cerr << "Error playing from " << time << " seconds: " << errorBuffer << std::endl;
    }
}

void FEHSound::platformOpenScaledFile()
{
    // Close current MCI device
    std::string command = "close " + m_alias;
    mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    
    // Open the scaled file
    command = "open \"" + tempFilePath + "\" type waveaudio alias " + m_alias;
    MCIERROR error = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (error != 0)
    {
        char errorBuffer[256];
        mciGetErrorStringA(error, errorBuffer, sizeof(errorBuffer));
        std::cerr << "Error opening scaled sound file: " << tempFilePath << " - MCI Error: " << errorBuffer << std::endl;
    }
}

#endif // _WIN32

// =============================================================================
// MACOS IMPLEMENTATION
// =============================================================================
#ifdef __APPLE__

void FEHSound::platformInit()
{
    // Initialize NSSound with original file
    CFStringRef cfStr = CFStringCreateWithCString(kCFAllocatorDefault, filepath.c_str(), kCFStringEncodingUTF8);
    if (!cfStr)
    {
        std::cerr << "Error converting filepath to CFStringRef: " << filepath << std::endl;
        nsSound = nullptr;
        return;
    }
    
    Class NSSoundClass = (Class)objc_getClass("NSSound");
    if (!NSSoundClass)
    {
        std::cerr << "Error: NSSound class not found." << std::endl;
        CFRelease(cfStr);
        nsSound = nullptr;
        return;
    }
    
    id soundAlloc = ((id(*)(Class, SEL))objc_msgSend)(NSSoundClass, sel_registerName("alloc"));
    SEL initSelector = sel_registerName("initWithContentsOfFile:byReference:");
    nsSound = ((id(*)(id, SEL, id, BOOL))objc_msgSend)(soundAlloc, initSelector, (id)cfStr, YES);
    
    if (!nsSound)
    {
        std::cerr << "Error initializing NSSound for file: " << filepath << std::endl;
    }
    
    CFRelease(cfStr);
    pausedTime = 0.0;
}

void FEHSound::platformCleanup()
{
    if (nsSound)
    {
        ((void (*)(id, SEL))objc_msgSend)(nsSound, sel_registerName("stop"));
        ((void (*)(id, SEL))objc_msgSend)(nsSound, sel_registerName("release"));
        nsSound = nullptr;
    }
}

void FEHSound::platformPlay()
{
    // Create new NSSound with scaled file
    if (nsSound)
    {
        ((void (*)(id, SEL))objc_msgSend)(nsSound, sel_registerName("stop"));
        ((void (*)(id, SEL))objc_msgSend)(nsSound, sel_registerName("release"));
    }
    
    CFStringRef cfStr = CFStringCreateWithCString(kCFAllocatorDefault, tempFilePath.c_str(), kCFStringEncodingUTF8);
    if (cfStr)
    {
        Class NSSoundClass = (Class)objc_getClass("NSSound");
        if (NSSoundClass)
        {
            id soundAlloc = ((id(*)(Class, SEL))objc_msgSend)(NSSoundClass, sel_registerName("alloc"));
            SEL initSelector = sel_registerName("initWithContentsOfFile:byReference:");
            nsSound = ((id(*)(id, SEL, id, BOOL))objc_msgSend)(soundAlloc, initSelector, (id)cfStr, YES);
            
            if (nsSound)
            {
                ((BOOL(*)(id, SEL))objc_msgSend)(nsSound, sel_registerName("play"));
            }
        }
        CFRelease(cfStr);
    }
}

void FEHSound::platformPause()
{
    if (nsSound)
    {
        // NSSound doesn't have pause - simulate by stopping
        ((void (*)(id, SEL))objc_msgSend)(nsSound, sel_registerName("stop"));
    }
}

void FEHSound::platformRestart()
{
    platformPlay(); // Same as play for macOS
}

void FEHSound::platformPlayFrom(double time)
{
    platformPlay();
    
    if (nsSound)
    {
        ((void (*)(id, SEL, double))objc_msgSend)(nsSound, sel_registerName("setCurrentTime:"), time);
    }
}

#endif // __APPLE__

// =============================================================================
// LINUX IMPLEMENTATION (TODO)
// =============================================================================
#ifdef __linux__

void FEHSound::platformInit()
{
    // TODO: Implement Linux audio initialization (ALSA/PulseAudio/PipeWire)
    std::cerr << "Linux audio support not yet implemented" << std::endl;
}

void FEHSound::platformCleanup()
{
    // TODO: Implement Linux audio cleanup
}

void FEHSound::platformPlay()
{
    // TODO: Implement Linux audio playback
    std::cerr << "Linux audio playback not yet implemented" << std::endl;
}

void FEHSound::platformPause()
{
    // TODO: Implement Linux audio pause
}

void FEHSound::platformRestart()
{
    // TODO: Implement Linux audio restart
}

void FEHSound::platformPlayFrom(double time)
{
    // TODO: Implement Linux audio playFrom
}

#endif // __linux__
