#include "global.h"

#define numPhoneme sizeof(s_phonemes)/sizeof(PHONEMET_T)


// Random number seed
uint8_t seed0;
uint8_t seed1;
uint8_t seed2;

u8 BUFFphonemes[128];   
u8 BUFFmodifier[128];   // must be same size as 'phonemes'
u8 g_text[64];

uint8_t default_pitch = 6;


//#ifdef _AVR_
   // Define routines to log to the UART
   


// Lookup user specified pitch changes
static const uint8_t PitchesP[]  = { 1, 2, 4, 6, 8, 10, 13, 16 };

/**
*
*  Find the single u8acter 'token' in 'vocab'
*  and append its phonemes to dest[x]
*
*  Return new 'x'
*/


/**
*
*   Convert phonemes to data string
*   Enter: textp = phonemes string
*   Return: phonemes = string of sound data
*         modifier = 2 bytes per sound data
*
*/


u8 phonemesToData(u8* textp, PHONEMET_T* phoneme){

   int phonemeOut = 0; // offset into the phonemes array
   int modifierOut = 0; // offset into the modifiers array
   unsigned int L81=0; // attenuate
   unsigned int L80=16;

   while(*textp != 0){
      // P20: Get next phoneme
      u8 anyMatch=0;
      int longestMatch=0;
      int ph;
      int numOut=0;   // The number of bytes copied to the output for the longest match
      

      // Get next phoneme, P2
      for(ph = 0; ph<numPhoneme; ph++){
         int numU8s;

         // Locate start of next phoneme 
         u8* ph_text = phoneme[ph].txt;

         
         // Set 'numU8s' to the number of u8acters
         // that we match against this phoneme
         for(numU8s=0;textp[numU8s]!=0 ;numU8s++){

            // get next input u8acter and make lower case
            u8 nextU8 = textp[numU8s];
            if(nextU8>='A' && nextU8<='Z'){
               nextU8 = nextU8 - 'A' + 'a';
            }

            if(nextU8!=ph_text[numU8s]){
               break;
            }
         }

         // if not the longest match so far then ignore
         if(numU8s <= longestMatch) continue;

         if(&ph_text[numU8s]!=0){
            // partial phoneme match
            continue;

         }

         // P7: we have matched the whole phoneme
         longestMatch = numU8s;

         // Copy phoneme data to 'phonemes'
         {
	   u8* ph_ph = (u8*)(&phoneme[ph].phoneme);
            for(numOut=0; &ph_ph[numOut]!= 0; numOut++){
               BUFFphonemes[phonemeOut+numOut] = ph_ph[numOut];
            }
         }
          L81 = phoneme[ph].attenuate+'0';
         anyMatch=1; // phoneme match found

         BUFFmodifier[modifierOut]=-1;
         BUFFmodifier[modifierOut+1]=0;

         // Get u8 from text after the phoneme and test if it is a numeric
         if(textp[longestMatch]>='0' && textp[longestMatch]<='9'){
            // Pitch change requested
            BUFFmodifier[modifierOut] = PitchesP[textp[longestMatch]-'1'];
            BUFFmodifier[modifierOut+1] = L81;
            longestMatch++;
         }

         // P10
         if(L81!='0' && L81 != L80 && BUFFmodifier[modifierOut]>=0){
            BUFFmodifier[modifierOut - 2] = BUFFmodifier[modifierOut];
            BUFFmodifier[modifierOut - 1] = '0';
            continue;
         } else {

            // P11
            if( (textp[longestMatch-1] ) == 0x20){
               // end of input string or a space
               //BUFFmodifier[modifierOut] = (modifierOut==0) ? 16 : BUFFmodifier[modifierOut-2];
               if(modifierOut == 0)
                  BUFFmodifier[modifierOut] = 16;
               else
                  BUFFmodifier[modifierOut] = BUFFmodifier[modifierOut-2];
            }
         }

      } // next phoneme

      // p13
      L80 = L81;
      if(longestMatch==0 && anyMatch==0){
         //loggerP(PSTR("Mistake in speech at "));
	//         sprintf(logtxt,"Mistake in speech at %s", textp); doslog(logtxt);
         //logger(textp);
         return 0;
      }

      // Move over the bytes we have copied to the output
      phonemeOut += numOut; 
      if(phonemeOut > sizeof(BUFFphonemes)-16){
	//         printf(logtxt,"Mistake Text too long");
         return 0;
      }

      // P16

      // Copy the modifier setting to each sound data element for this phoneme
      if(numOut > 2){
         int count;
         for(count=0; count != numOut; count+=2){
            BUFFmodifier[modifierOut + count + 2] = BUFFmodifier[modifierOut + count];
            BUFFmodifier[modifierOut + count + 3] = 0;
         }
      }
      modifierOut += numOut;

      //p21
      textp += longestMatch;
   }

   BUFFphonemes[phonemeOut++]='z';
   BUFFphonemes[phonemeOut++]='z';
   BUFFphonemes[phonemeOut++]='z';
   BUFFphonemes[phonemeOut++]='z';
   
   while(phonemeOut < sizeof(BUFFphonemes)){
      BUFFphonemes[phonemeOut++]=0;
   }

   while(modifierOut < sizeof(BUFFmodifier)){
      BUFFmodifier[modifierOut++]=-1;
      BUFFmodifier[modifierOut++]=0;
   }

   return 1; 
}


/*
   Generate a random number
*/
uint8_t random(void){
   uint8_t tmp = (seed0 & 0x48) + 0x38;
   seed0<<=1;
   if(seed1 & 0x80){
      seed0++;
   }
   seed1<<=1;
   if(seed2 & 0x80){
      seed1++;
   }
   seed2<<=1;
   if(tmp & 0x40){
      seed2++;
   }
   return seed0;
}


void soundOn(void){
   seed0=0xCEu;
   seed1=7;
   seed2=0xCFu;
}

// Logarithmic scale
//static const int16_t Volume[8] = {0,PWM_TOP * 0.01,PWM_TOP * 0.02,PWM_TOP * 0.03,PWM_TOP * 0.06,PWM_TOP * 0.12,PWM_TOP * 0.25,PWM_TOP * 0.5};

// Linear scale
//static const int16_t Volume[8] = {0,PWM_TOP * 0.07,PWM_TOP * 0.14,PWM_TOP * 0.21,PWM_TOP * 0.29,PWM_TOP * 0.36,PWM_TOP * 0.43,PWM_TOP * 0.5};
/*
static uint16_t Volume[8] = {0,
   56,
   112,
   168,
   232,
   288,
   344,
   400};
*/

uint8_t playTone(uint8_t soundNum, uint8_t soundPos, u8 pitch1, u8 pitch2, uint8_t count, uint8_t volume){
  //   uint8_t* soundData = &SNDSMP + ((int16)(soundNum) * 0x40);
   //int8 cmd = pgm_read_byte(&soundData[soundPos % 0x40]);
   
   //sprintf(logtxt, "value: %02X @ soundnum:%u", cmd, soundnum); doslog(logtxt);
   while(count-- > 0 ){
      int8_t s;
      
      //      s = pgm_read_byte(&soundData[soundPos & 0x3F]);
      //      sound((s & volume));
      //      pause(pitch1);
      //      sound(((s>>4) & volume));
      //      pause(pitch2);

      soundPos++;
   }
   return soundPos & 0x3f;
}

void play(uint8_t duration, uint8_t soundNumber){
   while(duration-- != 0){
      playTone(soundNumber,random(), 7,7, 10, 15);
   }
}


void setPitch(uint8_t pitch){
   default_pitch = pitch;
}


/*
*  Speak a string of phonemes
*/

/*
void speak(u8* textp){
   uint8_t 
      phonemeIn,            // offset into text
      byte2,
      modifierIn,            // offset into stuff in modifier
      punctuationPitchDelta;   // change in pitch due to fullstop or question mark
      
   u8 byte1;
   u8 phoneme;
   SOUND_INDEX_T* soundIndex;
   uint8_t sound1Num;         // Sound data for the current phoneme
   uint8_t sound2Num;         // Sound data for the next phoneme
   uint8_t sound2Stop;         // Where the second sound should stop
   u8 pitch1;         // pitch for the first sound
   u8 pitch2;         // pitch for the second sound
   short i;
   uint8_t sound1Duration;      // the duration for sound 1
   
   if(phonemesToData(textp,s_phonemes)){
      // phonemes has list of sound bytes


      soundOn();
      
      // _630C
      byte1=0;
      punctuationPitchDelta=0;

      //Q19
      for(phonemeIn=0,modifierIn=0;BUFFphonemes[phonemeIn]!=0; phonemeIn+=2, modifierIn+=2){
         uint8_t  duration;   // duration from text line
         uint8_t 	SoundPos;   // offset into sound data
         uint8_t 	fadeSpeed=0;

         phoneme=BUFFphonemes[phonemeIn];
         if(phoneme=='z'){
            delay(15);
            continue;
         }else if(phoneme=='#'){
            continue;
         }else{

            // Collect info on sound 1
            soundIndex = &SNDINDEX[phoneme - 'A'];
            sound1Num = pgm_read_byte(&soundIndex->SoundNumber);
            byte1 = pgm_read_byte(&soundIndex->byte1);
            byte2 = pgm_read_byte(&soundIndex->byte2);

            duration = BUFFphonemes[phonemeIn+1] - '0';   // Get duration from the input line
            if(duration!=1){
               duration<<=1;
            }
            duration += 6;                     // scaled duration from the input line (at least 6)

            sound2Stop = 0x40>>1;


            pitch1 = BUFFmodifier[modifierIn];
            if(BUFFmodifier[modifierIn + 1]==0 || pitch1==-1){
               pitch1 = 10;
               duration -= 6;
            }else if(BUFFmodifier[modifierIn + 1]=='0' || duration==6){
               duration -= 6;
            }
            

            //q8
            pitch2 = BUFFmodifier[modifierIn+2];
            if(BUFFmodifier[modifierIn + 3]==0 || pitch2 == -1){
               pitch2 = 10;
            }

            //q10

            if(byte1<0){
               sound1Num = 0;
               random();
               sound2Stop=(0x40>>1)+2;


            }else{
               // is positive
               if(byte1==2){
                  // 64A4
                  // Make a white noise sound !
                  uint8_t volume;               // volume mask

                  //volume = (duration==6) ? 15 : 1;  /// volume mask
                  
                  if(duration==6)
                     volume = 15;
                  else
                     volume = 1;
                  
                  for(duration <<= 2; duration>0; duration--){
                     playTone(sound1Num,random(),8,12,20, volume);
                     // Increase the volume
                     if(++volume==16){
                        volume = 15;   // full volume from now on
                     }

                  }
                  continue;

               }else{
                  //q11
                  if(byte1 != 0){
                     delay(25);
                  }
               }
            }

         }


         // 6186
         pitch1 += default_pitch + punctuationPitchDelta;
         if(pitch1<1){
            pitch1=1;
         }

         pitch2 += default_pitch + punctuationPitchDelta;
         if(pitch2<1){
            pitch2=1;
         }

         // get next phoneme
         phoneme=BUFFphonemes[phonemeIn + 2];

         if(phoneme==0 || phoneme=='z'){
            if(duration==1){
               delay(60);
            }
            phoneme='a';   // change to a pause
         }else{
            // s6
            if(byte2 != 1){
               byte2 = (byte2 + pgm_read_byte(&SoundIndex[phoneme-'A'].byte2))>>1;
            }

            if(byte1 < 0 || pgm_read_byte(&SoundIndex[phoneme-'A'].byte1) != 0){
               phoneme ='a'; // change to a pause
            }
         }

         // S10
         sound2Num = pgm_read_byte(&SoundIndex[phoneme-'A'].SoundNumber);

         sound1Duration = 0x80;         // play half of sound 1
         if(sound2Num==sound1Num){
            byte2 = duration;
         }

         // S11
         if( (byte2>>1) == 0 ){
            sound1Duration = 0xff;            // play all of sound 1
         }else{
            // The fade speed between the two sounds
            fadeSpeed = (sound1Duration + (byte2>>1))/byte2;

            if(duration==1){
               sound2Stop = 0x40;   // dont play sound2
               sound1Duration = 0xff;         // play all of sound 1
               pitch1 = 12;
            }
         }

         SoundPos = 0;
         do{
            uint8_t sound1Stop = (sound1Duration>>2) & 0x3fu;
            //uint8_t sound1End = min(sound1Stop , sound1Stop);   // min(a,b) (a<b) ? a:b
            
            uint8_t sound1end;
            
            if(!(sound1Stop<sound1Stop))
               sound1end = sound2stop; 

            if( sound1Stop != 0 ){
               SoundPos = playTone(sound1Num,SoundPos,pitch1,pitch1, (uint16_t)sound1End, 15);
            }

            // s18
            if(sound2Stop != 0x40){
               SoundPos = playTone(sound2Num,SoundPos,pitch2,pitch2, (uint16_t)(sound2Stop - sound1End), 15);
            }

            //s23
            if(sound1Duration!=0xff && duration<byte2){
               // Fade sound1 out
               sound1Duration -= fadeSpeed;
               if( sound1Duration >= (uint8_t)0xC8){
                  sound1Duration=0;   // stop playing sound 1
               }
            }


            // Call any additional sound
            if(byte1==-1){
               play(3,30);   // make an 'f' sound
            }else if(byte1==-2){
               play(3,29);   // make an 's' sound
            }else if(byte1==-3){
               play(3,33);   // make a 'th' sound
            }else if(byte1==-4){
               play(3,27);   // make a 'sh' sound
            }

         }while(--duration!=0);
       

         // Scan ahead to find a '.' or a '?' as this will change the pitch
         punctuationPitchDelta=0;
         for(i=6; i>0; i--){
            u8 next = BUFFphonemes[phonemeIn + (int16)(i * 2)];
            if(next=='i'){
               // found a full stop
               punctuationPitchDelta = 6 - i; // Lower the pitch
            }else if(next=='h'){
               // found a question mark
               punctuationPitchDelta = i - 6; // Raise the pitch
            }
         }

         if(byte1 == 1){
            delay(25);
         }


      } // next phoneme

   }
   soundOff();
}

*/
