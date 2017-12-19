/*******************************************************************************/
/* Bela Csound Rendering functions                                             */
/*                                                                             */
/*******************************************************************************/

#include <Bela.h>
#include <Midi.h>
#include <csound/csound.hpp>
#include <vector>
#include <sstream>

#define ANCHNS 8

static int OpenMidiInDevice(CSOUND *csound, void **userData, const char *dev);
static int CloseMidiInDevice(CSOUND *csound, void *userData);
static int ReadMidiData(CSOUND *csound, void *userData, unsigned char *mbuf,
			int nbytes);

struct CsChan {
  std::vector<MYFLT> data;
  std::stringstream name;
};

struct CsData {
  Csound *csound;
  int blocksize;
  int res;
  int count;
  CsChan channel[ANCHNS];
};

struct CsMIDI {
  Midi midi;
};
  
CsData gCsData;

bool setup(BelaContext *context, void *userData)
{
  Csound *csound;
  const char *csdfile = "my.csd"; /* CSD name */
  const char *midiDev = "-Mhw:1,0,0"; /* MIDI device */
  int numArgs = 8;
  const char *args[] = { "csound", csdfile, "-iadc", "-odac", "-+rtaudio=null",
			 "--realtime", "--daemon", midiDev};

  if(context->audioInChannels != context->audioOutChannels) {
    printf("Number of audio inputs != number of audio outputs.\n");
    return false;
  }

  /* setup Csound */
  csound = new Csound();
  csound->SetHostImplementedAudioIO(1,0);
  csound->SetHostImplementedMIDIIO(1);
  csound->SetExternalMidiInOpenCallback(OpenMidiInDevice);
  csound->SetExternalMidiReadCallback(ReadMidiData);
  csound->SetExternalMidiInCloseCallback(CloseMidiInDevice);
  gCsData.csound = csound;
  gCsData.res = csound->Compile(numArgs, args);
  gCsData.blocksize = csound->GetKsmps()*csound->GetNchnls();
  gCsData.count = 0;
  
  /* set up the channels */
  for(int i; i < ANCHNS; i++) {
    gCsData.channel[i].data.resize(csound->GetKsmps());
    gCsData.channel[i].name << "analogue" << i+1;
  }
  
  if(gCsData.res != 0) return false;
  else return true;
}

void render(BelaContext *context, void *Data)
{
  if(gCsData.res == 0) {
    int n,i,k,count, frmcount,blocksize,res;
    Csound *csound = gCsData.csound;
    MYFLT scal = csound->Get0dBFS();
    MYFLT* audioIn = csound->GetSpin();
    MYFLT* audioOut = csound->GetSpout();
    int nchnls = csound->GetNchnls();
    int chns = nchnls < context->audioOutChannels ?
      nchnls : context->audioOutChannels;
    int an_chns = context->analogInChannels > ANCHNS ?
      ANCHNS : context->analogInChannels;
    CsChan *channel = &(gCsData.channel[0]);
    float frm = 0, incr = ((float) context->analogFrames)/context->audioFrames;
    int an_chans = context->analogInChannels;
    count = gCsData.count;
    blocksize = gCsData.blocksize;
    

    /* this is called when Csound is not running */
    if(count < 0) {
      for(n = 0; n < context->audioFrames; n++){
	for(i = 0; i < context->audioOutChannels; i++){
	  audioWrite(context,n,i,0);
	}
      }
      return;
    }
   
    /* this is where Csound is called */
    for(n = 0; n < context->audioFrames; n++, frm+=incr, count+=nchnls){
      if(count == blocksize) {
	/* set the channels */
	for(i = 0; i < an_chns; i++) {
          csound->SetChannel(channel[i].name.str().c_str(),
			     &(channel[i].data[0]));
	}
	/* run csound */
	if((res = csound->PerformKsmps()) == 0) count = 0;
	else {
	  count = -1;
	  break;
	}
      }
      /* read/write audio data */
      for(i = 0; i < chns; i++){
	audioIn[count+i] = audioRead(context,n,i);
	audioWrite(context,n,i,audioOut[count+i]/scal);
      }
 
      /* read analogue data 
         analogue frame pos gets incremented according to the
         ratio analogFrames/audioFrames.
      */
      frmcount = count/nchnls;
      for(i = 0; i < an_chns; i++) {
	k = (int) frm;
        channel[i].data[frmcount] = analogRead(context,k,i);
      }	
    }
    gCsData.res = res;
    gCsData.count = count;
  }
}

void cleanup(BelaContext *context, void *Data)
{
  delete gCsData.csound;
}

/** MIDI Input functions 
 */
int OpenMidiInDevice(CSOUND *csound, void **userData, const char *dev) {
  CsMIDI *midiData = new CsMIDI;
  Midi &midi = midiData->midi;
  midi.readFrom(dev);
  midi.enableParser(false);
  *userData = (void *) midiData;
}

int CloseMidiInDevice(CSOUND *csound, void *userData) {
  CsMIDI *midiData = (CsMIDI *) userData;
  delete midiData;
}

int ReadMidiData(CSOUND *csound, void *userData,
		 unsigned char *mbuf, int nbytes) {
  int n = 0;
  CsMIDI *midiData = (CsMIDI *) userData;
  Midi &midi = midiData->midi;
  
  while((byte = midi.getInput()) >= 0) {
    *mbuf++ = (unsigned char) byte;
    if(++n == nbytes) break;
  }
  
  return n;				   
}
