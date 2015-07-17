#ifndef AUDIOEFFECTS_H
#define AUDIOEFFECTS_H

#include <QMediaPlayer>
#include <QObject>
#include <QMutex>
#include "bass.h"

#define STEPS 10  //steps of sound changing
#define MAX_S 60  //max speed
#define STEP_SP MAX_S/STEPS //speed step
#define BACK 400 //back for sound

#define TRACK_LEN 8000
#define PART_LEN TRACK_LEN/STEPS

class AudioEffects :public QObject
{
    Q_OBJECT
public:
    AudioEffects();
    ~AudioEffects();

public slots:
    void BassAudioProcess();

public:
    void RunSpeed(int speed);
    void changeBassSpeed();

    void stopFone();
    bool audioPlay = true;


private:
    QMutex audio_mtx;
    float vel = 1.0f;
    int prev_speed = 0, current_speed = 0, new_speed = 0;
     HSTREAM stream; /* дескриптор потока */
     int current_freq = 44100;

};

#endif // AUDIOEFFECTS_H
