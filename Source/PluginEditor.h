/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
enum Theme
{
	Rainbow,
	Light,
	Dark,
	Matrix
};

struct Properties
{
	juce::String recordingPath;
	Theme theme = Rainbow;
};

struct EditorState
{
	float startPos = 0.0f;
	float width = 0.0f;
	bool enable = false;
};

class ReSamplerAudioProcessorEditor : public juce::AudioProcessorEditor,
	public juce::DragAndDropContainer,
	public juce::Timer,
	public juce::OpenGLRenderer
{
public:
	ReSamplerAudioProcessorEditor(ReSamplerAudioProcessor&);
	~ReSamplerAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics&) override;
	void resized() override;

private:
	void paintRainbow(juce::Graphics& g);
	void paintLight(juce::Graphics& g);
	void paintDark(juce::Graphics& g);
	void paintMatrix(juce::Graphics& g);

	void prepareWaveform();
	void manageProperties();
	void saveState();
	void loadState();
	bool isInSelectedArea(const int pos);

	void timerCallback() override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseUp(const juce::MouseEvent& event) override;
	void mouseDoubleClick(const juce::MouseEvent& event) override;
	void mouseDrag(const juce::MouseEvent& event) override;
	void menuButtonClicked();

	void setTheme(Theme theme);
	void setBufferLength(int length);
	void setRecordingPath();

	void newOpenGLContextCreated() override { };
	void renderOpenGL() override { };
	void openGLContextClosing() override { };

	juce::OpenGLContext openGLContext;

	ReSamplerAudioProcessor& audioProcessor;
	juce::ComponentBoundsConstrainer constrainer;
	std::unique_ptr<juce::PropertiesFile> propertiesFile;
	Properties properties;
	EditorState editorState;
	int offset = 0;

	juce::AudioThumbnailCache thumbnailCache{ 6 };
	juce::AudioFormatManager formatManager;
	juce::AudioThumbnail waveform{ 44100, formatManager, thumbnailCache };

	juce::TextButton menuButton{ "Menu" };
	std::unique_ptr<juce::FileChooser> fileChooser;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReSamplerAudioProcessorEditor)
};
