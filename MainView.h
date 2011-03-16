/*
 * Copyright 2011, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		Alexandre Deckner <alex@zappotek.com>
 */
#ifndef _MAINVIEW_H
#define _MAINVIEW_H

#include <View.h>
#include <Rect.h>

struct media_format;

class MainView : public BView {
public:
					MainView(BRect frame);
					~MainView();
	virtual void	Draw(BRect updateRect);
	virtual void	AttachedToWindow();
	virtual void	FrameResized(float width, float height);
	
	void			_LoadWave();
	void			_ProcessAudio(char* buffer, media_format* format, int64 frameCount);
	float*			fWave;
	uint32			fWaveCursor;
	float			fAverage;
	uint32			fAverageCursor;
	uint32			fDownsamplingWindow;
	uint32			fDownsampleCount;	
	
	BRect			fWaveFrame;
};

#endif	// _MAINVIEW_H
