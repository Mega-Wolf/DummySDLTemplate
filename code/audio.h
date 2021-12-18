#pragma once

#include "macros.h"
#include "maths.h"

struct playing_audio {
    float VolumeLeft;
    float VolumeRight;
    int StartFrame;
    //int CurrentFrame;
    bool Loop;
    loaded_audio AudioFile;
};

#define MAX_AUDIO_CLIPS 1024

int AudioClipsCount;
playing_audio AudioClips[MAX_AUDIO_CLIPS];

unsigned int CurrentAudioFrame;

int IntOutput[2048][2];

void AudioClipStart(loaded_audio audio, bool loop, float volume) {
    // TODO(Tobi): Shall I do it like this?
    if (AudioClipsCount >= MAX_AUDIO_CLIPS) { return; }

    playing_audio* newAudioClip = &AudioClips[AudioClipsCount++];
    *newAudioClip = {};
    newAudioClip->AudioFile = audio;
    newAudioClip->Loop = loop;
    // TODO(Tobi): Maybe volume has to be changable afterwards; so I need a fixed position in the array -> Generation stuff
    // This could also be used to abort audio
    newAudioClip->VolumeLeft = volume;
    newAudioClip->VolumeRight = volume;
    newAudioClip->StartFrame = CurrentAudioFrame;
}

void AudioCallback(void *userdata, unsigned char *stream8, int len) {
    UNUSED_PARAM(userdata);

    // NOTE(Tobi): This wants me to write len chars to stream8
    // So I have to write less data

    int framesWanted = len / 2 / 2; // 2 because of short and 2 because of channel

    short (*outputFrames)[2] = (short(*)[2]) stream8;

    // Clear audio
    inc0 (frame_i,   framesWanted) {
        IntOutput[frame_i][0] = 0;
        IntOutput[frame_i][1] = 0;
    }

    inc0 (clip_i,   AudioClipsCount) {
        playing_audio* playing_audio_ = &AudioClips[clip_i];

        int frameInMusic = CurrentAudioFrame - playing_audio_->StartFrame;
        int restFrames = AtLeast(playing_audio_->AudioFile.Frames - frameInMusic, 0);

        // Add values up
        int framesToWrite = Min(restFrames, framesWanted);
        inc0 (frame_i,   framesToWrite) {
            IntOutput[frame_i][0] += (int) (playing_audio_->AudioFile.Data[frameInMusic + frame_i][0] * playing_audio_->VolumeLeft);
            IntOutput[frame_i][1] += (int) (playing_audio_->AudioFile.Data[frameInMusic + frame_i][1] * playing_audio_->VolumeRight);
        }

        if (restFrames < framesWanted) {
            if (playing_audio_->Loop) {
                int framesFromStart = framesWanted - restFrames;
                // TODO(Tobi): Looping sounds have to be at least WANTED FRAME TIME / 2 frames long

                inc0 (frame_i,   framesFromStart) {
                    IntOutput[frame_i + restFrames][0] += (int) (playing_audio_->AudioFile.Data[frame_i][0] * playing_audio_->VolumeLeft);
                    IntOutput[frame_i + restFrames][1] += (int) (playing_audio_->AudioFile.Data[frame_i][1] * playing_audio_->VolumeRight);
                }

                playing_audio_->StartFrame += playing_audio_->AudioFile.Frames;
            } else {
                AudioClips[clip_i--] = AudioClips[--AudioClipsCount];
                continue;
            }
        }
    }

    // Clamp values and write out
    inc0 (frame_i,   framesWanted) {
        outputFrames[frame_i][0] = (short) Clamp(IntOutput[frame_i][0], -32768, +32767);
        outputFrames[frame_i][1] = (short) Clamp(IntOutput[frame_i][1], -32768, +32767);
    }

    CurrentAudioFrame += framesWanted;
}
