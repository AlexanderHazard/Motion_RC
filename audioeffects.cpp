#include "audioeffects.h"

#include <QDebug>
#include <QThread>
#include <QApplication>

AudioEffects::AudioEffects()
{

QString pathToSounds = QApplication::applicationDirPath() +"/audio/rally.wav";

    if (BASS_Init (-1, current_freq, BASS_DEVICE_3D , 0, NULL)) {
        const char *filename = pathToSounds.toStdString().c_str();
        stream = BASS_StreamCreateFile(FALSE, filename, 0, 0, BASS_SAMPLE_LOOP);
     }
}

AudioEffects::~AudioEffects()
{
    audioPlay = false;
}



void AudioEffects::BassAudioProcess()
{
  while(audioPlay)
  {
   if(BASS_ChannelIsActive(stream) != BASS_ACTIVE_PLAYING) BASS_ChannelPlay(stream,TRUE);
   audio_mtx.lock();
   if(current_speed != new_speed) changeBassSpeed();
   audio_mtx.unlock();
   QThread::msleep(10);
  }
}

void AudioEffects::RunSpeed(int speed)
{
    audio_mtx.lock();

    new_speed = speed;

    audio_mtx.unlock();

}


void AudioEffects::changeBassSpeed()
{
    if(current_speed < new_speed) current_speed++;
     else if(current_speed > new_speed) current_speed--;

    int new_freq = current_freq + (2600*current_speed);
    qDebug() << "FReq" << new_freq;
    BASS_ChannelSetAttribute(stream, BASS_ATTRIB_FREQ, new_freq);

}

void AudioEffects::stopFone()
{
    //speed_sound->stop();
}
