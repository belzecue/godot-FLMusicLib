/* Game music emulator library C interface (also usable from C++) */

/* Game_Music_Emu 0.6.1 */
#ifndef GME_H
#define GME_H

#ifdef __cplusplus
	extern "C" {
#endif

#define GME_VERSION 0x000601 /* 1 byte major, 1 byte minor, 1 byte patch-level */

/* Error string returned by library functions, or NULL if no error (success) */
typedef const char* gme_err_t;

/* First parameter of most gme_ functions is a pointer to the Music_Emu */
typedef struct Music_Emu Music_Emu;


/******** Basic operations ********/

/* Create emulator and load game music file/data into it. Sets *out to new emulator. */
gme_err_t gme_open_file( const char path [], Music_Emu** out, int sample_rate );

/* Number of tracks available */
int gme_track_count( Music_Emu const* );

/* Start a track, where 0 is the first track */
gme_err_t gme_start_track( Music_Emu*, int index );

/* Generate 'count' 16-bit signed samples info 'out'. Output is in stereo. */
gme_err_t gme_play( Music_Emu*, int count, short out [] );

/* Finish using emulator and free memory */
void gme_delete( Music_Emu* );


/******** Track position/length ********/

/* Set time to start fading track out. Once fade ends track_ended() returns true.
Fade time can be changed while track is playing. */
void gme_set_fade( Music_Emu*, int start_msec );

/* True if a track has reached its end */
int gme_track_ended( Music_Emu const* );

/* Number of milliseconds (1000 = one second) played since beginning of track */
int gme_tell( Music_Emu const* );

/* Number of samples generated since beginning of track */
int gme_tell_samples( Music_Emu const* );

/* Seek to new time in track. Seeking backwards or far forward can take a while. */
gme_err_t gme_seek( Music_Emu*, int msec );

/* Equivalent to restarting track then skipping n samples */
gme_err_t gme_seek_samples( Music_Emu*, int n );


/******** Informational ********/

/* If you only need track information from a music file, pass gme_info_only for
sample_rate to open/load. */
enum { gme_info_only = -1 };

/* Most recent warning string, or NULL if none. Clears current warning after returning.
Warning is also cleared when loading a file and starting a track. */
const char* gme_warning( Music_Emu* );

/* Load m3u playlist file (must be done after loading music) */
gme_err_t gme_load_m3u( Music_Emu*, const char path [] );

/* Clear any loaded m3u playlist and any internal playlist that the music format
supports (NSFE for example). */
void gme_clear_playlist( Music_Emu* );

/* Gets information for a particular track (length, name, author, etc.).
Must be freed after use. */
typedef struct gme_info_t gme_info_t;
gme_err_t gme_track_info( Music_Emu const*, gme_info_t** out, int track );

/* Frees track information */
void gme_free_info( gme_info_t* );

struct gme_info_t
{
	/* times in milliseconds; -1 if unknown */
	int length;			/* total length, if file specifies it */
	int intro_length;	/* length of song up to looping section */
	int loop_length;	/* length of looping section */
	
	/* Length if available, otherwise intro_length+loop_length*2 if available,
	otherwise a default of 150000 (2.5 minutes). */
	int play_length;
	
	int i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15; /* reserved */
	
	/* empty string ("") if not available */
	const char* system;
	const char* game;
	const char* song;
	const char* author;
	const char* copyright;
	const char* comment;
	const char* dumper;
	
	const char *s7,*s8,*s9,*s10,*s11,*s12,*s13,*s14,*s15; /* reserved */
};


/******** Advanced playback ********/

/* Adjust stereo echo depth, where 0.0 = off and 1.0 = maximum. Has no effect for
GYM, SPC, and Sega Genesis VGM music */
void gme_set_stereo_depth( Music_Emu*, double depth );

/* Disable automatic end-of-track detection and skipping of silence at beginning
if ignore is true */
void gme_ignore_silence( Music_Emu*, int ignore );

/* Adjust song tempo, where 1.0 = normal, 0.5 = half speed, 2.0 = double speed.
Track length as returned by track_info() assumes a tempo of 1.0. */
void gme_set_tempo( Music_Emu*, double tempo );

/* Number of voices used by currently loaded file */
int gme_voice_count( Music_Emu const* );

/* Name of voice i, from 0 to gme_voice_count() - 1 */
const char* gme_voice_name( Music_Emu const*, int i );

/* Mute/unmute voice i, where voice 0 is first voice */
void gme_mute_voice( Music_Emu*, int index, int mute );

/* Set muting state of all voices at once using a bit mask, where -1 mutes all
voices, 0 unmutes them all, 0x01 mutes just the first voice, etc. */
void gme_mute_voices( Music_Emu*, int muting_mask );

/* Frequency equalizer parameters (see gme.txt) */
/* Implementers: If modified, also adjust Music_Emu::make_equalizer as needed */
typedef struct gme_equalizer_t
{
	double treble; /* -50.0 = muffled, 0 = flat, +5.0 = extra-crisp */
	double bass;   /* 1 = full bass, 90 = average, 16000 = almost no bass */
	
	double d2,d3,d4,d5,d6,d7,d8,d9; /* reserved */
} gme_equalizer_t;

/* Get current frequency equalizater parameters */
void gme_equalizer( Music_Emu const*, gme_equalizer_t* out );

/* Change frequency equalizer parameters */
void gme_set_equalizer( Music_Emu*, gme_equalizer_t const* eq );

/* Enables/disables most accurate sound emulation options */
void gme_enable_accuracy( Music_Emu*, int enabled );


/******** Game music types ********/

/* Music file type identifier. Can also hold NULL. */
typedef const struct gme_type_t_* gme_type_t;

/* Emulator type constants for each supported file type */
extern const gme_type_t
	gme_ay_type,
	gme_gbs_type,
	gme_gym_type,
	gme_hes_type,
	gme_kss_type,
	gme_nsf_type,
	gme_nsfe_type,
	gme_sap_type,
	gme_spc_type,
	gme_vgm_type,
	gme_vgz_type;

/* Type of this emulator */
gme_type_t gme_type( Music_Emu const* );

/* Pointer to array of all music types, with NULL entry at end. Allows a player linked
to this library to support new music types without having to be updated. */
gme_type_t const* gme_type_list();

/* Name of game system for this music file type */
const char* gme_type_system( gme_type_t );

/* True if this music file type supports multiple tracks */
int gme_type_multitrack( gme_type_t );


/******** Advanced file loading ********/

/* Error returned if file type is not supported */
extern const char* const gme_wrong_file_type;

/* Same as gme_open_file(), but uses file data already in memory. Makes copy of data. */
gme_err_t gme_open_data( void const* data, long size, Music_Emu** out, int sample_rate );

/* Determine likely game music type based on first four bytes of file. Returns
string containing proper file suffix (i.e. "NSF", "SPC", etc.) or "" if
file header is not recognized. */
const char* gme_identify_header( void const* header );

/* Get corresponding music type for file path or extension passed in. */
gme_type_t gme_identify_extension( const char path_or_extension [] );

/* Determine file type based on file's extension or header (if extension isn't recognized).
Sets *type_out to type, or 0 if unrecognized or error. */
gme_err_t gme_identify_file( const char path [], gme_type_t* type_out );

/* Create new emulator and set sample rate. Returns NULL if out of memory. If you only need
track information, pass gme_info_only for sample_rate. */
Music_Emu* gme_new_emu( gme_type_t, int sample_rate );

/* Load music file into emulator */
gme_err_t gme_load_file( Music_Emu*, const char path [] );

/* Load music file from memory into emulator. Makes a copy of data passed. */
gme_err_t gme_load_data( Music_Emu*, void const* data, long size );

/* Load music file using custom data reader function that will be called to
read file data. Most emulators load the entire file in one read call. */
typedef gme_err_t (*gme_reader_t)( void* your_data, void* out, int count );
gme_err_t gme_load_custom( Music_Emu*, gme_reader_t, long file_size, void* your_data );

/* Load m3u playlist file from memory (must be done after loading music) */
gme_err_t gme_load_m3u_data( Music_Emu*, void const* data, long size );


/******** User data ********/

/* Set/get pointer to data you want to associate with this emulator.
You can use this for whatever you want. */
void  gme_set_user_data( Music_Emu*, void* new_user_data );
void* gme_user_data( Music_Emu const* );

/* Register cleanup function to be called when deleting emulator, or NULL to
clear it. Passes user_data to cleanup function. */
typedef void (*gme_user_cleanup_t)( void* user_data );
void gme_set_user_cleanup( Music_Emu*, gme_user_cleanup_t func );


#ifdef __cplusplus
	}
#endif

#endif
