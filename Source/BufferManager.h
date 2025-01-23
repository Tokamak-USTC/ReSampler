/*
  ==============================================================================

	BufferManager.h
	Created: 19 Jan 2025 9:52:32pm
	Author:  Tokamak

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct BufferParameters
{
	int numChannels = 2;
	int sampleRate = 44100;
};

struct BufferState
{
	bool isRecording = true;
	bool isPlaying = false;
	int writePosition = 0;
	int readPosition = 0;
};

class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	void initializeBuffer(int numChannels, int sampleRate);
	void setBufferLength(int length);
	int getBufferLength() const { return bufferLength; }
	int getBufferSampleRate() const { return bufferParameters.sampleRate; }
	juce::AudioBuffer<float>* getBufferPointer() const { return recordBuffer.get(); }

	void writeToBuffer(const juce::AudioBuffer<float>& buffer);
	void readFromBuffer(juce::AudioBuffer<float>& buffer);

	BufferState bufferState;

private:
	int bufferLength = 30;
	BufferParameters bufferParameters;
	juce::CriticalSection bufferLock;
	std::unique_ptr<juce::AudioBuffer<float>> recordBuffer;
};

