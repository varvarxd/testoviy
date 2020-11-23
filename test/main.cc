//test
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <winsock.h>
#include <vector>

#include "AudioDriver.h"
#include "SoundSource.h"
#include "WaveGenerator.h"

#pragma pack(push, 1)
struct  MThdHeader{
	uint32_t type;
	uint32_t length;
	uint16_t format;
	uint16_t ntrks;
	uint16_t division;
};
struct MTrkHeader{
	uint32_t type;
	uint32_t length;
};
#pragma pack(pop)

int main(int, char**)
{
	static const double s_freq_table[] = {
			32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00,
			 51.91, 55.00, 58.27, 61.74, 65.41, 69.30, 73.42, 77.78,
			 82.41, 87.31, 92.50, 98.00, 103.83, 110.00, 116.54, 123.47,
			 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00,
			 207.65, 220.00, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13,
			 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88,
			 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99,
			 830.61, 880.00, 932.33, 987.77, 1046.50, 1108.73, 1174.66, 1244.51,
			 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760.00, 1864.66, 1975.53,
			 2093.00, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96,
			 3322.44, 3520.00, 3729.31, 3951.07, 4186.01, 4434.92, 4698.64, 4978.03,
			 5274.04, 5587.65, 5919.91, 6271.93, 6644.88, 7040.00, 7458.62, 7902.13,
			 8372.02, 8869.84, 9397.27, 9956.06, 10548.08, 11175.30, 11839.82, 12543.85,
			 13289.75, 14080.00, 14917.24, 15804.27, 16744.04, 17739.69, 18794.55, 19912.13,
			 21096.16, 22350.61, 23679.64, 25087.71, 26579.50, 28160.00, 29834.48, 31608.53,
			 33488.07, 35479.38, 37589.09, 39824.25, 42192.33, 44701.21, 47359.29, 50175.42
};


	SDL_Init(SDL_INIT_EVERYTHING);

	AudioDriver drv;
	drv.unpause();

	auto src = std::make_shared<SoundSource>();
	auto wg = std::make_shared<SquareWaveGenerator>();
	auto eg = std::make_shared<ConstantEnvelopeGenerator>(1.);
	auto fg = std::make_shared<FMWaveGenerator>();

	FMInstrument f;
	f.al = 0;
	f.fb = 20;
	f.op[0] = {0.300, 1., 0.95, 0.0001, 0.0001, 0.};
	f.op[1] = {0.300, 1., 0.95, 0.0001, 0.0001, 0.};
	f.op[2] = {0.300, 1., 0.95, 0.00001, 0.0001, 0.};
	f.op[3] = {0.300, 1., 0.95, 0.00001, 0.0001, 0.};
	f.be[0] = 0.5; f.be[1] = 0.5; f.be[2] = 1.; f.be[3] = 1.;
	f.multi[0] = 1.0; f.multi[1] = 0.5; f.multi[2] = 0.25; f.multi[3] = 0.5;

	fg->set_freq(440.);
	fg->setup(f);
	src->set_envelope_generator(eg);
	src->set_wave_generator(fg);

	drv.set_sound_source(src);
	drv.set_master_volume(4000);

	std::fstream file;
	file.open("MK2.mid", std::ios::in | std::ios::binary);

	MThdHeader mthd;
	file.read(reinterpret_cast<char*>(&mthd), sizeof(mthd));

	mthd.length = ntohs(mthd.length);
	mthd.format = ntohs(mthd.format);
	mthd.ntrks = ntohs(mthd.ntrks);
	mthd.division = ntohs(mthd.division);

	std::cout << "MIDI fail, format: " <<  mthd.format <<
			", дорожек: " << mthd.ntrks <<
			", division: " << mthd.division << std::endl;

	std::vector<std::vector<uint8_t>> track;
	track.resize(mthd.ntrks);
	for(unsigned i = 0; i < mthd.ntrks; i++){
		MTrkHeader mtrk;
		file.read(reinterpret_cast<char*>(&mtrk), sizeof(mtrk));
		mtrk.length = ntohl(mtrk.length);
		std::cout << "Дорожка " << i << " длина: " << mtrk.length << std::endl;
		track[i].resize(mtrk.length);
		file.read(reinterpret_cast<char*>(&track[i][0]), mtrk.length);
	}

	unsigned trk = 0;
	unsigned ptr = 0;

	std::cout << "------> Текущая дорожка: " << trk << std::endl;

	unsigned event_type = 0;

	std::cout << std::hex << std::setfill('0');

	while (ptr < track[trk].size()){
		unsigned delta = 0;
		unsigned delta_part;
		do{
			delta_part = track[trk][ptr++];
			delta <<= 7; delta+= delta_part & 0x7f;
		}while (delta_part & 0x80);

		if (delta > 0) SDL_Delay(delta);

		std::cout  << "Delta: " << delta << std::endl;

		unsigned new_event = track[trk][ptr++];
		if (new_event >= 0x80)
			event_type = new_event;
		else
			ptr--;

		switch(event_type) {

			case 0x80 ... 0x8f:{
				unsigned note = track[trk][ptr++];
				unsigned velocity = track[trk][ptr++];
				fg->set_freq(s_freq_table[note]);
				fg->set_volume(velocity/127.);
				fg->key_off();
			}break;

			case 0x90 ... 0x9f:{
				unsigned note = track[trk][ptr++];
				unsigned velocity = track[trk][ptr++];
				fg->set_freq(s_freq_table[note]);
				fg->set_volume(velocity/127.);
				std::cout << "Nota: " << note << ", Speed: " << velocity << std::endl;
				std::cout.flush();
				fg->key_on();
			}break;

			case 0xa0 ... 0xaf:{ //Polyphonic Key Pressure (Aftertouch).
				unsigned knum, pvalue;
				knum = track[trk][ptr++];
				pvalue = track[trk][ptr++];
				std::cout << "key (note) number: " <<
						std::setw(2) << knum << " pressure value: " <<
						std::setw(2) << pvalue << std::endl;
			}break;

			case 0xb0 ... 0xbf:{
				unsigned control, value;
				control = track[trk][ptr++];
				value = track[trk][ptr++];
				std::cout << "Control Change: " <<
						std::setw(2) << control << " to " <<
						std::setw(2) << value << std::endl;
			}break;

			case 0xc0 ... 0xcf:{
				unsigned program;
				program = track[trk][ptr++];
				std::cout << "Program Change: " <<
						std::setw(2) << program << std::endl;
			}break;

			case 0xd0 ... 0xdf:{
				unsigned aftertouch;
				aftertouch = track[trk][ptr++];
				std::cout << "Aftertouch: " <<
						std::setw(2) << aftertouch << std::endl;
			}break;

			case 0xe0 ... 0xef:{
				unsigned pitchWheel;
				pitchWheel = track[trk][ptr++];
				std::cout << "Pitch Wheel Change: " <<
						std::setw(2) << pitchWheel << std::endl;
			}break;

			case 0xf0:{
				unsigned manufacturerID;
				manufacturerID = track[trk][ptr++];
				if(manufacturerID==0){
					manufacturerID = (((manufacturerID << 8) | track[trk][ptr++]) << 8  | track[trk][ptr++]);
				}
				std::cout << "System Exclusive. manufacturer: " <<
						std::setw(2) << manufacturerID << " ID " << track[trk][ptr++]* 0x1U << std::endl;

				while(event_type!=0xf7)
					event_type = track[trk][ptr++];
				std::cout << "End of exclusive" << std::endl;
			}break;

			case 0xf2:{ //Song Position Pointer.
				unsigned songpp, msb;
				songpp = track[trk][ptr++];
				msb = track[trk][ptr++];
				std::cout << "Song Position Pointer LSB: " <<
						std::setw(2) << songpp << " MSB: " << msb << std::endl;
			}break;

			case 0xf3:{ //Song Select
				unsigned songselect;
				songselect = track[trk][ptr++];
				std::cout << "Номер песни: " <<
						std::setw(2) << songselect << std::endl;
			}break;

			case 0xf6:{ //Tune Request
				std::cout << "Tune Request." << std::endl;
			}break;

			case 0xf8:{
				std::cout << "Timing Clock. Sent 24 times per quarter note when synchronisation is required." << std::endl;
			}break;

			case 0xfa:{
				std::cout << "Start the current sequence playing." << std::endl;
			}break;

			case 0xfb:{
				std::cout << "Continue at the point the sequence was Stopped." << std::endl;
			}break;

			case 0xfc:{
				std::cout << "Stop the current sequence." << std::endl;
			}break;

			case 0xfe:{
				std::cout << "Active Sensing." << std::endl;
			}break;

			case 0xff:{
				unsigned meta_type = track[trk][ptr++];
				unsigned meta_len = track[trk][ptr++];
				switch (meta_type) {
					case 0x00:{
						uint16_t seqnum = 0;
						seqnum += track[trk][ptr++] * 0x100U;
						seqnum += track[trk][ptr++];
						std::cout << "Sequence number: " <<
						std::dec << seqnum << std::hex << std::endl;
					}break;

					case 0x01:{
						std::string text;
						while (meta_len --)
							text += char(track[trk][ptr++]);
						std::cout << "Text: " << text << std::endl;
					}break;

					case 0x02:{
						std::string cprt;
						while (meta_len --)
							cprt += char(track[trk][ptr++]);
						std::cout << " Copyright Notice: " << cprt << std::endl;
					}break;

					case 0x03:{
						std::string name;
						while (meta_len --)
							name += char(track[trk][ptr++]);
						std::cout << "Имя дорожки: " << name << std::endl;
					}break;

					case 0x04:{
						std::string InstrumentName;
						while (meta_len --)
							InstrumentName += char(track[trk][ptr++]);
						std::cout << "Instrument Name: " << InstrumentName << std::endl;
					}break;

					case 0x05:{
						std::string lyric;
						while (meta_len --)
							lyric += char(track[trk][ptr++]);
						std::cout << "lyric: " << lyric << std::endl;
					}break;

					case 0x06:{
						std::string Marker;
						while (meta_len --)
							Marker += char(track[trk][ptr++]);
						std::cout << "Marker: " << Marker << std::endl;
					}break;

					case 0x07:{
						std::string cuepoint;
						while (meta_len --)
							cuepoint += char(track[trk][ptr++]);
						std::cout << "Cue Point: " << cuepoint << std::endl;
					}break;

					case 0x20:{
						unsigned prefix;
						prefix = track[trk][ptr++];
						std::cout << "MIDI Channel Prefix: " << prefix << std::endl;
					}break;

					case 0x51:{
						unsigned temp1 = track[trk][ptr++];
						unsigned temp2 = track[trk][ptr++];
						unsigned temp3 = track[trk][ptr++];
						int result = (((temp1 << 8) |temp2) << 8 | temp3);
						std::cout << "Set Tempo: " << result << " микросекунд" << std::endl;
					}break;

					case 0x54:{
						unsigned hr = track[trk][ptr++];
	 					unsigned mn = track[trk][ptr++];
	 					unsigned se = track[trk][ptr++];
	 					unsigned fr = track[trk][ptr++];
	 					std::cout << "SMPTE Offset hr:" << hr
	 							<< " mn: " << mn
								<< " se: " << se
								<< " fr: " << fr << std::endl;

					}break;

					case 0x58:{
						unsigned nn = track[trk][ptr++];
	 					unsigned dd = track[trk][ptr++];
	 					unsigned num_tick = track[trk][ptr++];
	 					unsigned num_part = track[trk][ptr++];
	 					std::cout << "Размер " << nn << "/" << dd
	 							<< ", колличество тиков в ударе метронома " << num_tick
	 							<< ", колличество 32-х долей в " << num_tick << " тиках " << num_part << std::endl;

					}break;

					case 0x59:{
						unsigned sf = track[trk][ptr++];
						unsigned me = track[trk][ptr++];
						std::cout << "Key Signature sf: " << sf << " me: " << me << std::endl;
					}break;

					case 0x2f:
						std::cout << "---- Конец Дорожки ----" << std::endl;
						return 0;
					break;

					case 0x7f:{
						std::cout << "событие 0x7f" << std::endl;
					}break;

					default:
						std::cout << "Незамеченное мета событие " <<
						std::setw(2) << meta_type << std::endl;
						return 1;
				}
			}break;

			default:
				std::cout << "<!> Неизвестное событие: " <<
				std::setw(2) << event_type << " <!>" <<
				std::endl;
				return 1;

		}

	file.close();

	}

	return 0;
}
