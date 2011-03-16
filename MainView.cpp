/*
 * Copyright 2011, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 * 		Alexandre Deckner <alex@zappotek.com>
 */

#include "MainView.h"

#include <MediaFile.h>
#include <MediaTrack.h>
#include <Entry.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <new>

MainView::MainView(BRect frame)
	:
	BView(frame, "MainView", B_FOLLOW_ALL, B_WILL_DRAW),
	fWave(NULL),
	fWaveCursor(0),
	fAverage(0.0f),
	fAverageCursor(0),
	fWaveFrame(frame),
	fDownsamplingWindow(0),
	fDownsampleCount(0)
{
}


MainView::~MainView()
{
}


void
MainView::AttachedToWindow()
{
	BView::AttachedToWindow();
	bigtime_t start = system_time();
	_LoadWave();
	printf("LoadWave in %lims\n", (system_time() - start) / 1000 );
}


void
MainView::Draw(BRect updateRect)
{
	if (fWave == NULL || fWaveCursor == 0)
		return;

	float x = 0.0f;	
	float ycenter = fWaveFrame.Height() / 2.0f;
	float yscale = fWaveFrame.Height() / 2.0f;
	
	for (int32 i = 0; i < fDownsampleCount; i++) {
		float y = fWave[i] * yscale;
		StrokeLine(BPoint(x, ycenter - y), BPoint(x, ycenter + y));
		x += 1.0f;
	}
}


void
MainView::FrameResized(float width, float height)
{
}


void
MainView::_LoadWave() {
	int64 startFrame = 47000;
	int64 endFrame = 52000;
	fDownsampleCount = (uint32) fWaveFrame.Width() + 1;
	printf("Drawing samples = %u\n", fDownsampleCount);
	
	// instantiate a BMediaFile object, and make sure there was no error.
	BEntry entry("./song.wav");
	entry_ref ref;
	entry.GetRef(&ref);
	BMediaFile *mediaFile = new BMediaFile(&ref);
	status_t err = mediaFile->InitCheck();
	if (err != B_OK) {
		printf("cannot contruct BMediaFile object -- %s\n", strerror(err));
		return;
	}

	for (int32 i = 0; i < mediaFile->CountTracks(); i++) {
		BMediaTrack *track = mediaFile->TrackAt(i);
		if (track == NULL) {
			printf("cannot contruct BMediaTrack object\n");
			return;
		}

		media_format format;
		format.type = B_MEDIA_RAW_AUDIO;
		format.u.raw_audio.format = media_raw_audio_format::B_AUDIO_FLOAT;
		err = track->DecodedFormat(&format);
		if (err != B_OK) {
			printf("BMediaTrack::DecodedFormat error -- %s\n", strerror(err));
			return;
		}

		if (format.type == B_MEDIA_RAW_AUDIO) {
			// iterate through all the audio chunks and call the processing function

			char *buffer = (char *)malloc(format.u.raw_audio.buffer_size);
			int64 numFrames = endFrame - startFrame;//track->CountFrames();
			
			fWave = new(std::nothrow) float[fDownsampleCount];
			fDownsamplingWindow = numFrames / fDownsampleCount;
			printf("fWave realframes=%u\n", numFrames);
			printf("fWave downframes=%u\n", fDownsampleCount);
			printf("fWave window=%u\n", fDownsamplingWindow);
			//printf("fWave realframes=%u downframes=%u window=%u \n", numFrames, fDownsampleCount, fDownsamplingWindow);
			
			err = track->SeekToFrame(&startFrame);
			if (err) {
				printf("BMediaTrack::SeekToFrame(%li) error -- %s\n", startFrame, strerror(err));
				return;
			}
			
			int64 readFrameCount = 0;
			media_header mediaHeader;
			for(int64 j = 0; j < numFrames; j += readFrameCount) {
				err = track->ReadFrames(buffer, &readFrameCount, &mediaHeader);
				if (err) {
					printf("BMediaTrack::ReadFrames error -- %s\n", strerror(err));
					break;
				}
				_ProcessAudio(buffer, &format, readFrameCount);
				//printf("buffer\n");		
			}
			printf("fWaveCursor %ui\n", fWaveCursor);
			free(buffer);
		}

		mediaFile->ReleaseTrack(track);
	}
	delete mediaFile;
   
}


void
MainView::_ProcessAudio(char* buffer, media_format* format, int64 frameCount)
{		
	//printf("framecount %u\n", frameCount);
	
	uint32 channelCount = 2;
	// split input samples
	for (unsigned long frame = 0; frame < frameCount; frame++) {
		for (int c = 0; c < channelCount; c++) {
			float value = ((float *)buffer)[c + frame * channelCount];
			fAverage += value * value; // attention x 2 et au carré			
		}		
		
		if (fAverageCursor >= fDownsamplingWindow) {
			fAverage /= (float) fAverageCursor * (float) channelCount;
			fAverage = sqrt(fAverage);
			fWave[fWaveCursor] = fAverage;
			fWaveCursor++;
			
			fAverageCursor = 0;
			fAverage = 0;
		} else {		
			fAverageCursor++;
		}
	}
	
	
	
	
	//printf("fWaveCursor %ui\n", fWaveCursor);
	
	//printf("average = %f\n", average);	;
}
