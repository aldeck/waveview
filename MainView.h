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

struct frame_range {
	int64 left;
	int64 right;
	
	frame_range(int64 _left, int64 _right) : left(_left), right(_right) {};	
	int64 Width() const { return right - left; };
};

class MainView : public BView {
public:
					MainView(BRect frame);
					~MainView();
	virtual void	Draw(BRect updateRect);
	virtual void	AttachedToWindow();
	virtual void	FrameResized(float width, float height);
	virtual void	KeyDown(const char *bytes, int32 numBytes);
	
	void			_LoadWave();
	void			_ProcessAudio(char* buffer, media_format* format, int64 frameCount);
	float*			fWave;
	uint32			fDestCursor;
	uint32			fSourceCursor;
	float			fAverage;
	uint32			fAverageCursor;
	uint32			fDownsamplingWidth;
	uint32			fDownsampleCount;	
	
	BRect			fWaveFrame;
	frame_range 	fSourceWindow;
};

#endif	// _MAINVIEW_H
