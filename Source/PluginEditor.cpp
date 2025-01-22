/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


ReSamplerAudioProcessorEditor::ReSamplerAudioProcessorEditor(ReSamplerAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	startTimerHz(40);

	setConstrainer(&constrainer);
	constrainer.setMinimumSize(600, 75);

	setResizable(true, true);

	manageProperties();
	loadState();

	menuButton.onClick = [this] { menuButtonClicked(); };
	addAndMakeVisible(menuButton);

	formatManager.registerBasicFormats();

	openGLContext.setRenderer(this);
	openGLContext.attachTo(*this);
	openGLContext.setContinuousRepainting(true);
}

ReSamplerAudioProcessorEditor::~ReSamplerAudioProcessorEditor()
{
	openGLContext.detach();
	saveState();
}

//==============================================================================

void ReSamplerAudioProcessorEditor::paint(juce::Graphics& g)
{
	switch (properties.theme)
	{
	case Theme::Rainbow:
		menuButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
		menuButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
		paintRainbow(g);
		break;

	case Theme::Light:
		menuButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withBrightness(0.7f));
		menuButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black.withAlpha(0.7f));
		paintLight(g);
		break;

	case Theme::Dark:
		menuButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withBrightness(0.2f));
		menuButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
		paintDark(g);
		break;

	case Theme::Matrix:
		menuButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withBrightness(0.1f));
		menuButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
		paintMatrix(g);
		break;

	default:
		break;
	}
}

void ReSamplerAudioProcessorEditor::resized()
{
	menuButton.setBounds(getWidth() - 50, 10, 40, 20);
}

void ReSamplerAudioProcessorEditor::paintRainbow(juce::Graphics& g)
{
	int recLineX = getWidth() * (static_cast<float>(audioProcessor.bufferManager->bufferState.writePosition) / audioProcessor.bufferManager->getBufferPointer()->getNumSamples());

	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	//g.fillAll(juce::Colours::black.withBrightness(0.25f).interpolatedWith(juce::Colours::darkgreen, 0.05f));

	juce::ColourGradient waveBlock(juce::Colours::purple.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f), 0, 0,
		juce::Colours::purple.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f), getWidth(), 0, false);
	waveBlock.addColour(0.125, juce::Colours::red.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	waveBlock.addColour(0.25, juce::Colours::orange.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	waveBlock.addColour(0.375, juce::Colours::yellow.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	waveBlock.addColour(0.5, juce::Colours::green.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	waveBlock.addColour(0.625, juce::Colours::blue.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	waveBlock.addColour(0.75, juce::Colours::indigo.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	waveBlock.addColour(0.875, juce::Colours::violet.withAlpha(0.7f).interpolatedWith(juce::Colours::grey, 0.5f));
	g.setGradientFill(waveBlock);
	waveform.drawChannels(g, getLocalBounds(), 0, audioProcessor.bufferManager->getBufferLength(), 1.0f);

	juce::ColourGradient recBlock(juce::Colours::red.withAlpha(0.0f), recLineX - 49, 0,
		juce::Colours::red.withAlpha(0.5f), recLineX, 0, false);
	g.setGradientFill(recBlock);
	g.fillRect(recLineX - 50, 0, 49, getHeight());
	g.setColour(juce::Colours::red);
	g.fillRect(recLineX - 1, 0, 1, getHeight());

	g.setFont(20.0f);
	g.setColour(juce::Colours::white);
	g.drawText("ReSampler By Tokamak", getBounds(), juce::Justification::centred, true);

	if (!audioProcessor.bufferManager->bufferState.isRecording)
		g.fillAll(juce::Colours::orange.withAlpha(0.15f));

	if (editorState.enable)
	{
		g.setColour(juce::Colours::white.withAlpha(0.1f));
		g.fillRect(static_cast<int>(editorState.startPos * getWidth()), 0, static_cast<int>(editorState.width * getWidth()), getHeight());
	}
}

void ReSamplerAudioProcessorEditor::paintLight(juce::Graphics& g)
{
	int recLineX = getWidth() * (static_cast<float>(audioProcessor.bufferManager->bufferState.writePosition) / audioProcessor.bufferManager->getBufferPointer()->getNumSamples());

	g.fillAll(juce::Colours::black.withBrightness(0.7f));

	g.setColour(juce::Colours::black.withAlpha(0.7f));
	waveform.drawChannels(g, getLocalBounds(), 0, audioProcessor.bufferManager->getBufferLength(), 1.0f);

	juce::ColourGradient recBlock(juce::Colours::black.withAlpha(0.0f), recLineX - 49, 0,
		juce::Colours::black.withAlpha(0.5f), recLineX, 0, false);
	g.setGradientFill(recBlock);
	g.fillRect(recLineX - 50, 0, 49, getHeight());
	g.setColour(juce::Colours::black.withAlpha(0.7f));
	g.fillRect(recLineX - 1, 0, 1, getHeight());

	g.setFont(20.0f);
	g.drawText("ReSampler By Tokamak", getBounds(), juce::Justification::centred, true);

	if (!audioProcessor.bufferManager->bufferState.isRecording)
		g.fillAll(juce::Colours::orange.withAlpha(0.1f));

	if (editorState.enable)
	{
		g.setColour(juce::Colours::black.withAlpha(0.2f));
		g.fillRect(static_cast<int>(editorState.startPos * getWidth()), 0, static_cast<int>(editorState.width * getWidth()), getHeight());
	}
}

void ReSamplerAudioProcessorEditor::paintDark(juce::Graphics& g)
{
	int recLineX = getWidth() * (static_cast<float>(audioProcessor.bufferManager->bufferState.writePosition) / audioProcessor.bufferManager->getBufferPointer()->getNumSamples());

	g.fillAll(juce::Colours::black.withBrightness(0.15f));

	g.setColour(juce::Colours::black.withBrightness(0.7f));
	waveform.drawChannels(g, getLocalBounds(), 0, audioProcessor.bufferManager->getBufferLength(), 1.0f);

	juce::ColourGradient recBlock(juce::Colours::white.withBrightness(0.5f).withAlpha(0.0f), recLineX - 49, 0,
		juce::Colours::white.withBrightness(0.7f).withAlpha(0.7f), recLineX, 0, false);
	g.setGradientFill(recBlock);
	g.fillRect(recLineX - 50, 0, 49, getHeight());
	g.setColour(juce::Colours::white.withBrightness(0.9f));
	g.fillRect(recLineX - 1, 0, 1, getHeight());

	g.setFont(20.0f);
	g.drawText("ReSampler By Tokamak", getBounds(), juce::Justification::centred, true);

	if (!audioProcessor.bufferManager->bufferState.isRecording)
		g.fillAll(juce::Colours::white.withAlpha(0.1f));

	if (editorState.enable)
	{
		g.setColour(juce::Colours::white.withAlpha(0.1f));
		g.fillRect(static_cast<int>(editorState.startPos * getWidth()), 0, static_cast<int>(editorState.width * getWidth()), getHeight());
	}
}

void ReSamplerAudioProcessorEditor::paintMatrix(juce::Graphics& g)
{
	int recLineX = getWidth() * (static_cast<float>(audioProcessor.bufferManager->bufferState.writePosition) / audioProcessor.bufferManager->getBufferPointer()->getNumSamples());

	g.fillAll(juce::Colours::black.withBrightness(0.1f));

	float borderAlpha = 1.0f - static_cast<float>(recLineX) / getWidth();
	float borderBrightness = 0.8 * borderAlpha;

	juce::ColourGradient waveBlock(juce::Colours::darkgreen.withBrightness(borderBrightness).withAlpha(borderAlpha), 0, 0,
		juce::Colours::darkgreen.withBrightness(borderBrightness).withAlpha(borderAlpha), getWidth(), 0, false);
	waveBlock.addColour(static_cast<float>(recLineX) / getWidth(), juce::Colours::darkgreen.withBrightness(0.8f).withAlpha(1.0f));
	waveBlock.addColour(static_cast<float>(recLineX) / getWidth(), juce::Colours::darkgreen.withBrightness(0.0f).withAlpha(0.0f));
	g.setGradientFill(waveBlock);
	waveform.drawChannels(g, getLocalBounds(), 0, audioProcessor.bufferManager->getBufferLength(), 1.0f);

	juce::ColourGradient recBlock(juce::Colours::darkgreen.withAlpha(0.0f), recLineX - 49, 0,
		juce::Colours::darkgreen.withBrightness(0.8f).withAlpha(0.5f), recLineX, 0, false);
	g.setGradientFill(recBlock);
	g.fillRect(recLineX - 50, 0, 49, getHeight());
	g.setColour(juce::Colours::darkgreen.withBrightness(1.0f));
	g.fillRect(recLineX - 1, 0, 1, getHeight());

	g.setColour(juce::Colours::darkgreen.interpolatedWith(juce::Colours::white, 0.2f).withBrightness(1.0f));
	g.setFont(20.0f);
	g.drawText("ReSampler By Tokamak", getBounds(), juce::Justification::centred, true);

	if (!audioProcessor.bufferManager->bufferState.isRecording)
		g.fillAll(juce::Colours::darkgreen.interpolatedWith(juce::Colours::black, 0.2f).withAlpha(0.1f));

	if (editorState.enable)
	{
		g.setColour(juce::Colours::darkgreen.withAlpha(0.3f));
		g.fillRect(static_cast<int>(editorState.startPos * getWidth()), 0, static_cast<int>(editorState.width * getWidth()), getHeight());
	}
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

bool ReSamplerAudioProcessorEditor::isInSelectedArea(const int pos)
{
	if (pos <= editorState.startPos * getWidth() + 1)
		return false;
	if (pos >= (editorState.startPos + editorState.width) * getWidth() - 1)
		return false;
	return true;
}

void ReSamplerAudioProcessorEditor::timerCallback()
{
	if (waveform.getTotalLength() == 0.0)
		waveform.setSource(audioProcessor.bufferManager->getBufferPointer(), audioProcessor.bufferManager->getBufferSampleRate(), audioProcessor.bufferManager->getBufferLength());
	prepareWaveform();
	repaint();

}

void ReSamplerAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
	if (event.eventComponent == this)
	{
		if (event.mods.isLeftButtonDown() && !isInSelectedArea(event.getMouseDownX()) && editorState.enable)
		{
			editorState.enable = false;
			repaint();
		}
	}
}

void ReSamplerAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
}

void ReSamplerAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
	if (event.mods.isLeftButtonDown() && event.eventComponent == this)
	{
		audioProcessor.bufferManager->bufferState.isRecording = !audioProcessor.bufferManager->bufferState.isRecording;
		repaint();
	}
}

void ReSamplerAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
	if (event.mods.isLeftButtonDown() && event.eventComponent == this)
	{
		if (editorState.enable && isInSelectedArea(event.getMouseDownX()))
		{
			DBG("render audio file");
		}
		else
		{
			editorState.enable = true;
			if (event.getDistanceFromDragStartX() < 0)
			{
				editorState.startPos = static_cast<float>(event.getMouseDownX() + event.getDistanceFromDragStartX()) / getWidth();
				editorState.width = static_cast<float>(-event.getDistanceFromDragStartX()) / getWidth();
			}
			else
			{
				editorState.startPos = static_cast<float>(event.getMouseDownX()) / getWidth();
				editorState.width = static_cast<float>(event.getDistanceFromDragStartX()) / getWidth();
			}
			if (editorState.startPos < 0.0f)
			{
				editorState.width += editorState.startPos;
				editorState.startPos = 0.0f;
			}
			if (editorState.startPos + editorState.width > 1.0f)
				editorState.width = 1.0f - editorState.startPos;
			repaint();
		}
	}
}

void ReSamplerAudioProcessorEditor::menuButtonClicked()
{
	juce::PopupMenu menu;
	juce::PopupMenu theme;
	juce::PopupMenu bufferLength;

	theme.addItem("Rainbow", true, properties.theme == Theme::Rainbow, [this] {setTheme(Theme::Rainbow); });
	theme.addItem("Light", true, properties.theme == Theme::Light, [this] {setTheme(Theme::Light); });
	theme.addItem("Dark", true, properties.theme == Theme::Dark, [this] {setTheme(Theme::Dark); });
	theme.addItem("Matrix", true, properties.theme == Theme::Matrix, [this] {setTheme(Theme::Matrix); });


	bufferLength.addItem("15s", true, audioProcessor.bufferManager->getBufferLength() == 15, [this] {setBufferLength(15); });
	bufferLength.addItem("30s", true, audioProcessor.bufferManager->getBufferLength() == 30, [this] {setBufferLength(30); });
	bufferLength.addItem("60s", true, audioProcessor.bufferManager->getBufferLength() == 60, [this] {setBufferLength(60); });
	bufferLength.addItem("2min", true, audioProcessor.bufferManager->getBufferLength() == 120, [this] {setBufferLength(120); });
	bufferLength.addItem("5min", true, audioProcessor.bufferManager->getBufferLength() == 300, [this] {setBufferLength(300); });
	//bufferLength.addItem("10min", true, audioProcessor.bufferManager->getBufferLength() == 600, [this] {setBufferLength(600); });

	menu.addSubMenu("BufferLength", bufferLength);
	menu.addSubMenu("Theme", theme);
	menu.addItem("SetRecordingPath", [this] {setRecordingPath(); });

	menu.showMenuAsync(juce::PopupMenu::Options());
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

void ReSamplerAudioProcessorEditor::setTheme(Theme theme)
{
	if (properties.theme == theme)
		return;
	properties.theme = theme;
	saveState();
	repaint();
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





