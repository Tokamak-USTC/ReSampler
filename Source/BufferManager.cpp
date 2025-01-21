/*
  ==============================================================================

	BufferManager.cpp
	Created: 19 Jan 2025 9:52:52pm
	Author:  Tokamak

  ==============================================================================
*/

#include "BufferManager.h"

BufferManager::BufferManager()
{
}

BufferManager::~BufferManager()
{
}

void BufferManager::initializeBuffer(int numChannels, int sampleRate)
{
	//ÉèÖÃbuffer²ÎÊý
	bufferParameters.numChannels = numChannels;
	bufferParameters.sampleRate = sampleRate;

	//¶ÁÈ¡ÅäÖÃÎÄ¼þ
	juce::PropertiesFile::Options options;
	options.applicationName = "TKRS";
	options.filenameSuffix = ".settings";
	options.folderName = (juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getFullPathName() + "\\ReSampler").toStdString();
	options.storageFormat = juce::PropertiesFile::storeAsXML;
	juce::PropertiesFile propertiesFile(options);
	if (propertiesFile.containsKey("bufferLength"))
	{
		setBufferLength(propertiesFile.getIntValue("bufferLength"));
	}
	else
		setBufferLength(30);
}

void BufferManager::setBufferLength(int length)
{
	juce::ScopedLock lock(bufferLock);
	bufferLength = length;

	if (recordBuffer == nullptr)
	{
		recordBuffer = std::make_unique<juce::AudioBuffer<float>>(bufferParameters.numChannels, bufferLength * bufferParameters.sampleRate);
		recordBuffer->clear();
	}
	else
	{
		recordBuffer->setSize(bufferParameters.numChannels, bufferLength * bufferParameters.sampleRate);
		recordBuffer->clear();
	}
}

void BufferManager::writeToBuffer(const juce::AudioBuffer<float>& buffer)
{
	juce::ScopedLock lock(bufferLock);
	if (recordBuffer == nullptr || bufferState.isRecording == false)
		return;

	int numChannels = buffer.getNumChannels();
	int numSamples = buffer.getNumSamples();
	
	if (bufferState.writePosition + numSamples > recordBuffer->getNumSamples())
	{
		int overlap = bufferState.writePosition + numSamples - recordBuffer->getNumSamples();
		for (int channel = 0; channel < numChannels; channel++)
		{
			recordBuffer->copyFrom(channel, bufferState.writePosition, buffer, channel, 0, recordBuffer->getNumSamples() - bufferState.writePosition);
			recordBuffer->copyFrom(channel, 0, buffer, channel, recordBuffer->getNumSamples() - bufferState.writePosition, overlap);
		}
		bufferState.writePosition = overlap;
	}
	else
	{
		for (int channel = 0; channel < numChannels; channel++)
		{
			recordBuffer->copyFrom(channel, bufferState.writePosition, buffer, channel, 0, numSamples);
		}
		bufferState.writePosition += numSamples;
	}
}

void BufferManager::readFromBuffer(juce::AudioBuffer<float>& buffer)
{

}
