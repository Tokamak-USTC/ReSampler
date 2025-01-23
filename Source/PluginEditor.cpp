/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include <chrono>
#include <iomanip>
#include <sstream>
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
	editorState.startPosAbs = editorState.startPos * getWidth();
	editorState.widthAbs = editorState.width * getWidth();
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

	if (editorState.enableSelectArea)
	{
		g.setColour(juce::Colours::white.withAlpha(0.1f));
		g.fillRect(editorState.startPosAbs, 0, editorState.widthAbs, getHeight());
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

	if (editorState.enableSelectArea)
	{
		g.setColour(juce::Colours::black.withAlpha(0.2f));
		g.fillRect(editorState.startPosAbs, 0, editorState.widthAbs, getHeight());
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

	if (editorState.enableSelectArea)
	{
		g.setColour(juce::Colours::white.withAlpha(0.1f));
		g.fillRect(editorState.startPosAbs, 0, editorState.widthAbs, getHeight());
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

	if (editorState.enableSelectArea)
	{
		g.setColour(juce::Colours::darkgreen.withAlpha(0.3f));
		g.fillRect(editorState.startPosAbs, 0, editorState.widthAbs, getHeight());
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
	//加载theme
	if (propertiesFile->containsKey("theme"))
		properties.theme = static_cast<Theme>(propertiesFile->getIntValue("theme"));
	else
		properties.theme = Theme::Rainbow;

	//加载size
	if (propertiesFile->containsKey("width") && propertiesFile->containsKey("height"))
	{
		int width = propertiesFile->getIntValue("width");
		int height = propertiesFile->getIntValue("height");
		setSize(width, height);
	}
	else
		setSize(800, 100);

	//加载recordingPath
	if (propertiesFile->containsKey("recordingPath"))
		properties.recordingPath = propertiesFile->getValue("recordingPath");
	else
		properties.recordingPath = (juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getFullPathName() + "\\ReSampler\\Recordings").toStdString();
	juce::File recordingPath(properties.recordingPath);
	if (!recordingPath.exists())
		recordingPath.createDirectory();

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

bool ReSamplerAudioProcessorEditor::isInSelectedArea(const int pos)
{
	/*if (pos <= editorState.startPos * getWidth() + 1)
		return false;
	if (pos >= (editorState.startPos + editorState.width) * getWidth() - 1)
		return false;
	return true;*/
	return (pos > editorState.startPosAbs && pos < editorState.startPosAbs + editorState.widthAbs);
}

juce::String ReSamplerAudioProcessorEditor::exportSelectedArea()
{
	juce::File recordingDir(properties.recordingPath);
	if (!recordingDir.exists())
		recordingDir.createDirectory();

	auto now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm = *std::localtime(&now_time_t);
	std::ostringstream oss;
	oss << std::put_time(&now_tm, "%Y%m%d_%H%M%S");

	juce::String filePath = properties.recordingPath + "\\" + "TKRS_" + oss.str() + ".wav";
	juce::File audioFile(filePath);

	const juce::AudioBuffer<float>& buffer = *audioProcessor.bufferManager->getBufferPointer();
	int startSample = static_cast<int>(editorState.startPos * buffer.getNumSamples());
	if (startSample < 0)
		startSample = 0;
	int numSamples = static_cast<int>(editorState.width * buffer.getNumSamples());
	if (startSample + numSamples >= buffer.getNumSamples())
		numSamples = buffer.getNumSamples() - startSample;

	DBG("startSample: " << startSample << " numSamples: " << numSamples << " totalSamples " << buffer.getNumSamples());
	renderBuffer(buffer, startSample, numSamples, audioProcessor.bufferManager->getBufferSampleRate(), audioFile);

	return filePath;
}

void ReSamplerAudioProcessorEditor::renderBuffer(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples, double sampleRate, const juce::File& file)
{
	// 创建一个 WAV 格式的音频格式对象
	juce::WavAudioFormat wavFormat;
	std::unique_ptr<juce::FileOutputStream> fileStream(file.createOutputStream());

	if (fileStream != nullptr)
	{
		// 创建一个音频格式写入器
		std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(fileStream.get(),
			sampleRate,
			buffer.getNumChannels(),
			24,
			{},
			0));

		if (writer != nullptr)
		{
			// 将文件流的所有权转移给写入器
			fileStream.release();

			// 创建一个临时缓冲区，用于存储要写入的音频数据
			juce::AudioBuffer<float> tempBuffer(buffer.getNumChannels(), numSamples);

			// 从原始缓冲区中复制指定区域的数据到临时缓冲区
			for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
			{
				tempBuffer.copyFrom(channel, 0, buffer.getReadPointer(channel, startSample), numSamples);
			}

			// 将临时缓冲区中的数据写入文件
			writer->writeFromAudioSampleBuffer(tempBuffer, 0, numSamples);
		}
	}
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
		if (event.mods.isLeftButtonDown())
		{
			if (isInSelectedArea(event.getMouseDownX()) && editorState.enableSelectArea)
				editorState.dragFlag = true;
			if (!isInSelectedArea(event.getMouseDownX()) && editorState.enableSelectArea)
				editorState.enableSelectArea = false;
		}
	}
}

void ReSamplerAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
	if (event.eventComponent == this)
	{
		if (event.mods.isLeftButtonDown())
		{
			editorState.dragFlag = false;
			if (isInSelectedArea(event.getMouseDownX()))
				editorState.enableSelectArea = false;
		}

	}
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
		if (editorState.enableSelectArea && isInSelectedArea(event.getMouseDownX()))
		{
			if (editorState.dragFlag)
			{
				editorState.dragFlag = false;

				auto filePath = exportSelectedArea();
				juce::StringArray files;
				files.add(filePath);
				juce::DragAndDropContainer::performExternalDragDropOfFiles(files, true);
			}
		}
		else
		{
			if (event.getDistanceFromDragStartX() < 0)
			{
				editorState.startPosAbs = event.getMouseDownX() + event.getDistanceFromDragStartX();
				editorState.widthAbs = -event.getDistanceFromDragStartX();
				/*editorState.startPos = static_cast<float>(event.getMouseDownX() + event.getDistanceFromDragStartX()) / getWidth();
				editorState.width = static_cast<float>(-event.getDistanceFromDragStartX()) / getWidth();*/
			}
			else
			{
				editorState.startPosAbs = event.getMouseDownX();
				editorState.widthAbs = event.getDistanceFromDragStartX();
				/*editorState.startPos = static_cast<float>(event.getMouseDownX()) / getWidth();
				editorState.width = static_cast<float>(event.getDistanceFromDragStartX()) / getWidth();*/
			}
			if (editorState.startPosAbs < 0)
			{
				editorState.widthAbs += editorState.startPosAbs;
				editorState.startPosAbs = 0;
				/*editorState.width += editorState.startPos;
				editorState.startPos = 0.0f;*/
			}
			if (editorState.startPos + editorState.widthAbs >= getWidth())
				editorState.widthAbs = getWidth() - editorState.startPosAbs;
			editorState.startPos = static_cast<float>(editorState.startPosAbs) / getWidth();
			editorState.width = static_cast<float>(editorState.widthAbs) / getWidth();
			editorState.enableSelectArea = true;
			repaint();
		}
	}
}

void ReSamplerAudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{
	if (editorState.enableSelectArea && isInSelectedArea(event.getPosition().getX()))
		setMouseCursor(juce::MouseCursor::DraggingHandCursor);
	else
		setMouseCursor(juce::MouseCursor::NormalCursor);
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

void ReSamplerAudioProcessorEditor::setBufferLength(int length)
{
	if (audioProcessor.bufferManager->getBufferLength() == length)
		return;
	audioProcessor.bufferManager->setBufferLength(length);
	waveform.setSource(audioProcessor.bufferManager->getBufferPointer(), audioProcessor.bufferManager->getBufferSampleRate(), length);
	editorState.enableSelectArea = false;
	saveState();
	repaint();
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







