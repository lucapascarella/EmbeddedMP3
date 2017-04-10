/* 
 * File:   CCPlay.cpp
 * Author: luca
 * 
 * Created on April 10, 2017, 11:49 AM
 */

#include "MP3/CCPlay.h"

CCPlay::CCPlay() {
    sm = MP3_CCPLAY_HOME;
    //memset(play.filename, sizeof (play.filename), '\0');
}

CCPlay::CCPlay(const CCPlay& orig) {
}

CCPlay::~CCPlay() {
}

