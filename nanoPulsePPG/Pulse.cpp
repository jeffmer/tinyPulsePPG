/*
* Filter output from MAX30102 and detect heartbeat
*
* j.n.magee 15-10-2019
*/
#include "Pulse.h"

Pulse::Pulse(){
    cycle_max = 20;
    cycle_min = -20;
    positive = false;
    prev_sig = 0;
    amplitude_avg_total = 0;
}


//  Returns true if a beat is detected
bool Pulse::isBeat(int16_t signal) {
  bool beat = false;
  //while positive slope record maximum
  if (positive && (signal > prev_sig)) cycle_max = signal;  
  //while negative slope record minimum
  if (!positive && (signal < prev_sig)) cycle_min = signal;
  //  positive to negative i.e peak so declare beat
  if (positive && (signal < prev_sig)) {
    int amplitude = cycle_max - cycle_min;
    if (amplitude > 20 && amplitude < 3000) {
      beat = true;
      amplitude_avg_total += (amplitude - amplitude_avg_total/4); 
    }
    cycle_min = 0; positive = false;
  } 
  //negative to positive i.e valley bottom 
  if (!positive && (signal > prev_sig)) {
     cycle_max= 0; positive = true;
  } 
  prev_sig = signal; // save signal
  return beat;
}
