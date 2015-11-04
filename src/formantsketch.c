////////////from tinysynth:

/*
-each phoneme has 3xfreq(u8),3xw(u8)=res

'o',		12, 15, 0,		10, 10, 0, 

*/

// so set up is u8 freq[3], u8 res[3],???

const int SAMPLE_FREQUENCY		= 48000;
const float PI					= 3.1415926535f;
const float PI_2				= 2*PI;


for ( int f = 0; f < 3; f++ ) {
  char ff = p->f[f]; // the three freqs
  float freq = (float)ff*(50.0f/SAMPLE_FREQUENCY);

  float buf1Res = 0, buf2Res = 0;
  float q = 1.0f - p->w[f] * (PI * 10.0f / SAMPLE_FREQUENCY);
  float xp = 0;

  for ( int s = 0; s < howmany; s++ ) {
    // x is our float sample
    // Apply formant filter
    x = x + 2.0f * cosf ( PI_2 * freq ) * buf1Res * q - buf2Res * q * q;
    buf2Res = buf1Res;
    buf1Res = x;
    x = 0.75f * xp + x;
    xp = x;
    *(b++) += x; // round and round - bit must be floats
    *(b++) += x;
  }
 }




///////////FROM STK: onezero/onepole then 4 filter steps

//struct:

  FormSwep filters_[4];
  OnePole  onepole_;
  OneZero  onezero_;

// tick=processing

temp = onepole_.tick( onezero_.tick( voiced_->tick() ) ); //OneZero//OnePole.h

lastFrame_[0] = filters_[0].tick(temp); //Filter.h
  lastFrame_[0] += filters_[1].tick(temp);
  lastFrame_[0] += filters_[2].tick(temp);
  lastFrame_[0] += filters_[3].tick(temp);



{
  for ( int i=0; i<4; i++ )
    filters_[i].setSweepRate( 0.001 );
    
  onezero_.setZero( -0.9 );
  onepole_.setPole( 0.9 );
    
  noiseEnv_.setRate( 0.001 );
  noiseEnv_.setTarget( 0.0 );
    
  this->setPhoneme( "eee" );
  this->clear();
}  


bool VoicForm :: setPhoneme( const char *phoneme )
{
  bool found = false;
  unsigned int i = 0;
  while( i < 32 && !found ) {
    if ( !strcmp( Phonemes::name(i), phoneme ) ) {
      found = true;
      filters_[0].setTargets( Phonemes::formantFrequency(i, 0), Phonemes::formantRadius(i, 0), pow(10.0, Phonemes::formantGain(i, 0 ) / 20.0) );
      filters_[1].setTargets( Phonemes::formantFrequency(i, 1), Phonemes::formantRadius(i, 1), pow(10.0, Phonemes::formantGain(i, 1 ) / 20.0) );
      filters_[2].setTargets( Phonemes::formantFrequency(i, 2), Phonemes::formantRadius(i, 2), pow(10.0, Phonemes::formantGain(i, 2 ) / 20.0) );
      filters_[3].setTargets( Phonemes::formantFrequency(i, 3), Phonemes::formantRadius(i, 3), pow(10.0, Phonemes::formantGain(i, 3 ) / 20.0) );
      this->setVoiced( Phonemes::voiceGain( i ) );
      this->setUnVoiced( Phonemes::noiseGain( i ) );
    }
    i++;
  }

void VoicForm :: setFilterSweepRate( unsigned int whichOne, StkFloat rate )
{
  if ( whichOne > 3 ) {
    oStream_ << "VoicForm::setFilterSweepRate: filter select argument outside range 0-3!";
    handleError( StkError::WARNING ); return;
  }

  filters_[whichOne].setSweepRate(rate);
}


