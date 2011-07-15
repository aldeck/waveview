/*
 * Copyright 2011, Alexandre Deckner (alex@zappotek.com)
 * Distributed under the terms of the MIT License.
 *
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
	BView(frame, "MainView", B_FOLLOW_ALL, B_WILL_DRAW | B_SUBPIXEL_PRECISE),
	fWave(NULL),
	fDestCursor(0),
	fSourceCursor(0),
	fAverage(0.0f),
	fAverageCursor(0),
	fWaveFrame(frame),
	fDownsamplingWidth(0),
	fDownsampleCount(0),
	fSourceWindow(1470000, 1520000)
{
}


MainView::~MainView()
{
}


void
MainView::AttachedToWindow()
{
	BView::AttachedToWindow();
	_LoadWave();
	MakeFocus();
}


void
MainView::Draw(BRect updateRect)
{
	if (fWave == NULL || fDestCursor == 0)
		return;

	float x = 0.0f;
	float ycenter = fWaveFrame.Height() / 2.0f;
	float yscale = fWaveFrame.Height() / 2.0f;

	if (fDownsamplingWidth > 10) {
		for (int32 i = 0; i < (int32) fWaveFrame.Width(); i++) {
			float y = fWave[i] * yscale;

			StrokeLine(BPoint(x, ycenter - y), BPoint(x, ycenter + y));
			x += 1.0f;
		}
	} else {
		BPoint point(0, ycenter);
		MovePenTo(point);
		for (int32 i = 0; i < (int32) fWaveFrame.Width(); i++) {
			float y = ycenter + fWave[i] * yscale;
			StrokeLine(BPoint(x, y));
			x += 1.0f;
		}
	}

}


void
MainView::FrameResized(float width, float height)
{
}


void
MainView::_LoadWave() {
	bigtime_t startLoadWave = system_time();

	fDestCursor = 0;
	fSourceCursor = 0;
	fAverage = 0.0f;
	fAverageCursor = 0;
	fDownsamplingWidth = 0;
	fDownsampleCount = 0;

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

			// get the actual sourceWindow.left obtained by seeking
			bigtime_t start = system_time();
			int64 actualSeekFrame = fSourceWindow.left;
			err = track->SeekToFrame(&actualSeekFrame);
			if (err) {
				printf("BMediaTrack::SeekToFrame(%lli) error -- %s\n", fSourceWindow.left, strerror(err));
				return;
			}
			printf("BMediaTrack::SeekToFrame(%lli) seekedto=%lli in %llims\n", fSourceWindow.left, actualSeekFrame, (system_time() - start) / 1000);
			fSourceWindow.left = actualSeekFrame;

			frame_range destWindow(0, (uint64) fWaveFrame.Width());
			int64 totalSourceFrameCount = track->CountFrames();
			fDownsamplingWidth = fSourceWindow.Width() / destWindow.Width();

			printf("Source window left=%lli right=%lli width=%lli (total=%lli)\n", fSourceWindow.left, fSourceWindow.right, fSourceWindow.Width(), totalSourceFrameCount);
			printf("Dest window left=%lli right=%lli width=%lli\n", destWindow.left, destWindow.right, destWindow.Width());
			printf("Downsampling width=%lli\n", fDownsamplingWidth);

			delete [] fWave;
			fWave = NULL;
			fWave = new(std::nothrow) float[fSourceWindow.Width() / fDownsamplingWidth]; // ATT: on en prend plus que la largeur de dest car downsampling width entier
			char *buffer = new(std::nothrow) char[format.u.raw_audio.buffer_size];

			int64 readFrameCount = 0;
			media_header mediaHeader;
			fSourceCursor = fSourceWindow.left;
			for(int64 j = 0; j < fSourceWindow.Width(); j += readFrameCount) {

				err = track->ReadFrames(buffer, &readFrameCount, &mediaHeader);
				if (err) {
					printf("BMediaTrack::ReadFrames error -- %s\n", strerror(err));
					delete [] buffer;
					break;
				}

				if (fSourceCursor + readFrameCount >= fSourceWindow.right) {
					readFrameCount = fSourceWindow.right - fSourceCursor;
					printf("yes readFrameCount = %lli\n", readFrameCount);
				}

				_ProcessAudio(buffer, &format, readFrameCount);
			}
			printf("Source cursor %li (read %li)\n", fSourceCursor, fSourceCursor - fSourceWindow.left);
			printf("Dest cursor %li\n", fDestCursor);

			delete [] buffer;
		}

		mediaFile->ReleaseTrack(track);
	}
	delete mediaFile;


	printf("LoadWave in %lims\n", (system_time() - startLoadWave) / 1000 );
}


void
MainView::_ProcessAudio(char* buffer, media_format* format, int64 frameCount)
{
	//printf("framecount %u\n", frameCount);



	uint32 channelCount = 2;

	for (int64 frame = 0; frame < frameCount; frame++) {
		for (int c = 0; c < channelCount; c++) {
			float value = ((float *)buffer)[c + frame * channelCount];
			if (fDownsamplingWidth > 10)
				fAverage += value * value; // attention x 2 et au carré
			else
				fAverage += value;
		}

		if (fAverageCursor == fDownsamplingWidth - 1) {
			fAverage /= (float) fAverageCursor * (float) channelCount;
			if (fDownsamplingWidth > 10)
				fAverage = sqrt(fAverage);
			fWave[fDestCursor] = fAverage;
			fDestCursor++;

			fAverageCursor = 0;
			fAverage = 0;
		} else {
			fAverageCursor++;
		}
		fSourceCursor++;
	}




	//printf("fWaveCursor %ui\n", fWaveCursor);

	//printf("average = %f\n", average);	;
}


void MainView::KeyDown(const char *bytes, int32 numBytes)
{
   printf("ho\n");
   if ( numBytes >= 1 ) {
   		int64 width = fSourceWindow.Width();
      switch ( bytes[0] ) {
      case B_UP_ARROW:
         fSourceWindow.left += width / 10;
         fSourceWindow.right -= width / 10;
         _LoadWave();
         Invalidate();
         break;
       case B_DOWN_ARROW:
         fSourceWindow.left -= width / 10;
         fSourceWindow.right += width / 10;
         _LoadWave();
         Invalidate();
         break;
      case B_RIGHT_ARROW:
         fSourceWindow.left += width / 10;
         fSourceWindow.right += width / 10;
         _LoadWave();
         Invalidate();
         break;
      case B_LEFT_ARROW:
      	 fSourceWindow.left -= width / 10;
         fSourceWindow.right -= width / 10;
         _LoadWave();
         Invalidate();
         break;
      default:
         BView::KeyDown(bytes, numBytes);
         break;
      }
   }
}
