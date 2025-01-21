/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReSamplerAudioProcessorEditor::ReSamplerAudioProcessorEditor(ReSamplerAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	startTimerHz(24);

	setConstrainer(&constrainer);
	constrainer.setMinimumSize(600, 75);
	setResizable(true, true);

	manageProperties();
	loadState();

	menuButton.onClick = [this] { menuButtonClicked(); };
	addAndMakeVisible(menuButton);

	formatManager.registerBasicFormats();
}

ReSamplerAudioProcessorEditor::~ReSamplerAudioProcessorEditor()
{
	saveState();
}

//==============================================================================

void ReSamplerAudioProcessorEditor::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(juce::Colours::black);

	g.setColour(juce::Colours::white);

	waveform.drawChannels(g, getLocalBounds(), 0, audioProcessor.bufferManager->getBufferLength(), 1.0f);
	//g.setFont(15.0f);
	//g.drawFittedText("BufferLength: " + juce::String(audioProcessor.bufferManager->getBufferLength()), getLocalBounds(), juce::Justification::centred, 1);
	//g.drawFittedText("RecordingPath: " + properties.recordingPath, getLocalBounds(), juce::Justification::centred, 1);
}

void ReSamplerAudioProcessorEditor::resized()
{
	menuButton.setBounds(getWidth() - 50, 10, 40, 20);
}

void ReSamplerAudioProcessorEditor::prepareWaveform()
{
	if (waveform.getTotalLength() > 0.0)
	{
		if (audioProcessor.bufferManager->bufferState.writePosition - offset < 0) // overlap
		{
			waveform.addBlock(audioProcessor.bufferManager->getBufferPointer()->getNumSamples(), *(audioProcessor.bufferManager->getBufferPointer()), offset, audioProcessor.bufferManager->getBufferPointer()->getNumSamples() - offset);
			waveform.addBlock(audioProcessor.bufferManager->getBufferPointer()->getNumSamples(), *(audioProcessor.bufferManager->getBufferPointer()), 0, audioProcessor.bufferManager->bufferState.writePosition);
		}
		else
			waveform.addBlock(audioProcessor.bufferManager->getBufferPointer()->getNumSamples(), *(audioProcessor.bufferManager->getBufferPointer()), offset, audioProcessor.bufferManager->bufferState.writePosition - offset);
		offset = audioProcessor.bufferManager->bufferState.writePosition;
	}
}

void ReSamplerAudioProcessorEditor::manageProperties()
{
	juce::PropertiesFile::Options options;
	options.applicationName = "TKRS";
	options.filenameSuffix = ".settings";
	options.folderName = (juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getFullPathName() + "\\ReSampler").toStdString();
	options.storageFormat = juce::PropertiesFile::storeAsXML;
	propertiesFile = std::make_unique<juce::PropertiesFile>(options);
}

void ReSamplerAudioProcessorEditor::saveState()
{
	propertiesFile->setValue("width", getWidth());
	propertiesFile->setValue("height", getHeight());

	propertiesFile->setValue("theme", static_cast<int>(properties.theme));

	propertiesFile->setValue("recordingPath", properties.recordingPath);

	propertiesFile->setValue("bufferLength", audioProcessor.bufferManager->getBufferLength());


	propertiesFile->saveIfNeeded();
}

void ReSamplerAudioProcessorEditor::loadState()
{
	//加载size
	if (propertiesFile->containsKey("width") && propertiesFile->containsKey("height"))
	{
		int width = propertiesFile->getIntValue("width");
		int height = propertiesFile->getIntValue("height");
		setSize(width, height);
	}
	else
		setSize(800, 100);

	//加载theme
	if (propertiesFile->containsKey("theme"))
		properties.theme = static_cast<Theme>(propertiesFile->getIntValue("theme"));
	else
		properties.theme = Theme::Rainbow;

	//加载recordingPath
	if (propertiesFile->containsKey("recordingPath"))
		properties.recordingPath = propertiesFile->getValue("recordingPath");
	else
		properties.recordingPath = (juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getFullPathName() + "\\ReSampler\\Recordings").toStdString();


}

void ReSamplerAudioProcessorEditor::timerCallback()
{
	if (waveform.getTotalLength() == 0.0)
		waveform.setSource(audioProcessor.bufferManager->getBufferPointer(), audioProcessor.bufferManager->getBufferSampleRate(), audioProcessor.bufferManager->getBufferLength());
	prepareWaveform();
	repaint();
}

void ReSamplerAudioProcessorEditor::menuButtonClicked()
{
	juce::PopupMenu menu;
	juce::PopupMenu theme;
	juce::PopupMenu bufferLength;

	theme.addItem("Rainbow", true, properties.theme == Theme::Rainbow, [this] {setTheme(Theme::Rainbow); });
	theme.addItem("Light", true, properties.theme == Theme::Light, [this] {setTheme(Theme::Light); });
	theme.addItem("Dark", true, properties.theme == Theme::Dark, [this] {setTheme(Theme::Dark); });

	bufferLength.addItem("15s", true, audioProcessor.bufferManager->getBufferLength() == 15, [this] {setBufferLength(15); });
	bufferLength.addItem("30s", true, audioProcessor.bufferManager->getBufferLength() == 30, [this] {setBufferLength(30); });
	bufferLength.addItem("60s", true, audioProcessor.bufferManager->getBufferLength() == 60, [this] {setBufferLength(60); });
	bufferLength.addItem("2min", true, audioProcessor.bufferManager->getBufferLength() == 120, [this] {setBufferLength(120); });
	bufferLength.addItem("5min", true, audioProcessor.bufferManager->getBufferLength() == 300, [this] {setBufferLength(300); });
	bufferLength.addItem("10min", true, audioProcessor.bufferManager->getBufferLength() == 600, [this] {setBufferLength(600); });

	menu.addSubMenu("Theme", theme);
	menu.addSubMenu("BufferLength", bufferLength);
	menu.addItem("SetRecordingPath", [this] {setRecordingPath(); });

	menu.showMenuAsync(juce::PopupMenu::Options());
}

void ReSamplerAudioProcessorEditor::setTheme(Theme theme)
{
	if (properties.theme == theme)
		return;
	properties.theme = theme;
	saveState();
	repaint();
}

void ReSamplerAudioProcessorEditor::setRecordingPath()
{
	fileChooser = std::make_unique<juce::FileChooser>("Select a folder to save recordings",
		juce::File(properties.recordingPath),
		"*",
		true,
		true,
		nullptr);
	fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
		[this](const juce::FileChooser& fc)
		{
			if (fc.getResult().isDirectory())
			{
				properties.recordingPath = fc.getResult().getFullPathName();
				saveState();
				repaint();
			}
		});
}

void ReSamplerAudioProcessorEditor::setBufferLength(int length)
{
	if (audioProcessor.bufferManager->getBufferLength() == length)
		return;
	audioProcessor.bufferManager->setBufferLength(length);
	waveform.setSource(audioProcessor.bufferManager->getBufferPointer(), audioProcessor.bufferManager->getBufferSampleRate(), length);
	saveState();
	repaint();
}





