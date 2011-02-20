/*
 * CIMFPlayer.cpp
 *
 *  Created on: 17.02.2011
 *      Author: gerhard
 */

#include "CIMFPlayer.h"
#include "sdl/sound/CSound.h"
#include "sdl/sound/IMFPlayer.h"
#include "fileio/ResourceMgmt.h"
#include "fileio/compression/CHuffman.h"
#include "FindFile.h"
#include <fstream>
#include <string>

const Uint32 GalaxySongAssignments[] =
{
	0x02F206,
	0x030679,
	0x03103D,
};

CIMFPlayer::CIMFPlayer(const std::string& filename, const SDL_AudioSpec& AudioSpec) :
m_AudioDevSpec(AudioSpec),
m_opl_emulator(*g_pSound->getOPLEmulatorPtr()),
m_numreadysamples(0),
m_soundTimeCounter(5),
m_samplesPerMusicTick(m_AudioDevSpec.freq / m_opl_emulator.getIMFClockRate()),
m_TimeCount(0),
m_IMFReadTimeCount(0),
m_mix_buffer(new Sint32[m_samplesPerMusicTick])
{
    // Load the IMF File here!
	FILE *fp;
	word data_size;

	if( ( fp = OpenGameFile(filename, "rb") ) == NULL )
    	return;

	fread( &data_size, sizeof(word), 1, fp);
    if (data_size == 0) // Is the IMF file of Type-0?
    {
        fseek(fp, 0, SEEK_END);
        data_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }


	if(!m_IMF_Data.empty())
		m_IMF_Data.clear();
    const word imf_chunks = data_size/sizeof(IMFChunkType);
    m_IMF_Data.reserve(imf_chunks);

    if( imf_chunks != fread( m_IMF_Data.getStartPtr(), sizeof(IMFChunkType), imf_chunks, fp ) )
    	// TODO: Warn here that the file is corrupt!
    	fclose(fp);
    else
    	fclose(fp);
}

/**
 * \brief 	This function will load music using other dictionaries which are embedded in the Exe File.
 * 			Only galaxy supports that feature, and the original games will read two files form the EXE-file
 * 			AUDIOHED and AUDIODICT to get the right tune for the music player.
 */
CIMFPlayer::CIMFPlayer(const CExeFile& ExeFile, const int level, const SDL_AudioSpec &AudioSpec) :
m_AudioDevSpec(AudioSpec),
m_opl_emulator(*g_pSound->getOPLEmulatorPtr()),
m_numreadysamples(0),
m_soundTimeCounter(5),
m_samplesPerMusicTick(m_AudioDevSpec.freq / m_opl_emulator.getIMFClockRate()),
m_TimeCount(0),
m_IMFReadTimeCount(0),
m_mix_buffer(new Sint32[m_samplesPerMusicTick])
{
	const int episode = ExeFile.getEpisode();

	if(m_AudioDevSpec.format != 0)
	{
		// Open the Huffman dictionary and get AUDIODICT
		CHuffman Huffman;
		Huffman.readDictionaryNumber( ExeFile, 0 );

		/// First get the size of the AUDIO.CK? File.
		uint32_t audiofilecompsize;
		std::string init_audiofilename = "AUDIO.CK" + itoa(episode);

		std::string audiofilename = getResourceFilename( init_audiofilename, ExeFile.getDataDirectory(), true, false);

		if( audiofilename == "" )
			return;

		std::ifstream AudioFile;
		OpenGameFileR(AudioFile, audiofilename);

		// Read File Size and allocate memory so we can read it
		AudioFile.seekg( 0, std::ios::end );
		audiofilecompsize = AudioFile.tellg();
		AudioFile.seekg( 0, std::ios::beg );

		// create memory so we can store the Audio.ck there and use it later for extraction
		uint8_t *AudioCompFileData = new uint8_t[audiofilecompsize];
		AudioFile.read((char*)AudioCompFileData, audiofilecompsize);
		AudioFile.close();

		// Open the AUDIOHED so we know where tomp_IMF_Data decompress
		uint32_t number_of_audiorecs = 0;
		// That size must appear as integer in the ExeFile. Look for it!
		uint32_t *audiohedptr = (uint32_t*) ExeFile.getHeaderData();
		bool found = false;
		for( const uint32_t *endptr = (uint32_t*) ExeFile.getHeaderData()+ExeFile.getExeDataSize()/sizeof(uint32_t);
				audiohedptr < endptr ;
				audiohedptr++ )
		{
			if(*audiohedptr == audiofilecompsize)
			{
				for( const uint32_t *startptr = (uint32_t*) ExeFile.getHeaderData() ;
						audiohedptr > startptr ; audiohedptr-- )
				{
					found = true;
					// Get the number of Audio files we have
					number_of_audiorecs++;
					if(*audiohedptr == 0x0)
						break;
				}
				break;
			}
		}

		if(!found)
			return ;

		// Find the start of the embedded IMF files
		bool gap_detected = false;
		uint32_t music_start = 0;

		for( uint32_t slot = 0 ; slot<number_of_audiorecs ; slot++ )
		{
			const uint32_t audio_start = audiohedptr[slot];
			const uint32_t audio_end = audiohedptr[slot+1];

			if(!gap_detected && audio_start == audio_end)
				gap_detected = true;

			if(gap_detected && audio_start != audio_end)
			{
				music_start = slot+1;
				break;
			}
		}

		// Now get the proper music slot reading the assignment table.
		Uint16 music_order = 0;

		memcpy( &music_order, ExeFile.getRawData()+GalaxySongAssignments[episode-4]+level*sizeof(Uint16), sizeof(Uint16));

		/// Now we have all the data we need.
		// decompress every file of AUDIO.CK? using huffman decompression algorithm
		uint32_t slot = music_start+music_order;
		const uint32_t audio_start = audiohedptr[slot];
		const uint32_t audio_end = audiohedptr[slot+1];

		if( audio_start < audio_end )
		{
			const uint32_t audio_comp_data_start = audio_start+sizeof(uint32_t);
			const uint32_t *AudioCompFileData32 = (uint32_t*) (AudioCompFileData + audio_start);
			const uint32_t data_size = *AudioCompFileData32;

			if(!m_IMF_Data.empty())
				m_IMF_Data.clear();
		    const word imf_chunks = data_size/sizeof(IMFChunkType);
			m_IMF_Data.reserve(imf_chunks);
			Huffman.expand( (byte*)(AudioCompFileData+audio_comp_data_start), (byte*) m_IMF_Data.getStartPtr(), audio_end-audio_comp_data_start, data_size);
		}
	}
}

CIMFPlayer::~CIMFPlayer()
{
	if(m_mix_buffer)
		delete [] m_mix_buffer;
}

bool CIMFPlayer::open()
{
	return (!m_IMF_Data.empty());
}

void CIMFPlayer::close()
{
	if(m_numreadysamples)
		m_opl_emulator.Chip__GenerateBlock2( m_numreadysamples, m_mix_buffer );

	play(false);
	m_IMF_Data.gotoStart();
	return;
}

void CIMFPlayer::OPLUpdate(Sint16 *buffer, unsigned int length)
{
	m_opl_emulator.Chip__GenerateBlock2( length, m_mix_buffer );

    // Mix into the destination buffer, doubling up into stereo.
    for (unsigned int i=0; i<length; ++i)
    {
    	for (unsigned int j=0; j<m_AudioDevSpec.channels; j++)
    	{
    		*buffer = m_mix_buffer[i];
    		buffer++;
    	}
    }
}

void CIMFPlayer::readBuffer(Uint8* buffer, Uint32 length)
{
	if(!m_playing)
		return;

	/// if a delay of the instruments is pending, play it
    Uint32 sampleslen = length/(m_AudioDevSpec.channels*sizeof(Sint16));

    // while the waveform is not filled
    while(1)
    {
        if(m_numreadysamples)
        {
            if(m_numreadysamples < sampleslen)
            {
            	// Every time a tune has been played call this.
            	OPLUpdate( (Sint16*) buffer, m_numreadysamples);
            	buffer += m_numreadysamples*m_AudioDevSpec.channels*sizeof(Sint16);
                sampleslen -= m_numreadysamples;
            }
            else
            {
            	// Read the last stuff left in the emulators buffer. At this point the stream buffer is nearly full
            	OPLUpdate( (Sint16*) buffer, sampleslen);
            	m_numreadysamples -= sampleslen;
                break;
            }
        }

        do
        {
            if(m_IMFReadTimeCount > m_TimeCount) break;

        	// read the next instrument play to to the OPL Emulator and put a new delay
        	const IMFChunkType Chunk = m_IMF_Data.getNextElement();
        	m_IMFReadTimeCount = m_TimeCount + Chunk.Delay;
        	m_opl_emulator.Chip__WriteReg( Chunk.al_reg, Chunk.al_dat );
        }while(!m_IMF_Data.atStart());

        m_TimeCount++;

        m_numreadysamples = m_samplesPerMusicTick;
    }
}
