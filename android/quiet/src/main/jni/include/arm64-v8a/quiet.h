#ifndef QUIET_H
#define QUIET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* Representation for single sample containing sound */
typedef float quiet_sample_t;

typedef enum {
    quiet_success,
    quiet_mem_fail,
    quiet_encoder_bad_config,
    quiet_profile_malformed_json,
    quiet_profile_missing_key,
    quiet_profile_invalid_profile,
    quiet_msg_size,
    quiet_would_block,
    quiet_timedout,
    quiet_io,
} quiet_error;

/* Get last error set by libquiet
 *
 * quiet_get_last_error retrieves the last error set. If
 * libquiet was compiled with pthread, then this error will be specific
 * to the thread where this is called.
 */
quiet_error quiet_get_last_error();

/* cldoc:begin-category(options) */

/* DC-blocking filter options
 *
 * This DC blocker is applied near the end of the signal chain so that any
 * leftover DC component is removed. This is important for audio signals as we
 * do not want to send any DC out to speakers
 *
 * transfer function H(z)=(1 - (z^-1))/(1 - (1-alpha)*(z^-1))
 */
typedef struct { float alpha; } quiet_dc_filter_options;

/* Resampler options
 *
 * Controls arbitrary resample unit used by libquiet after generating 44.1kHz
 * signal or before decoding signal
 *
 * This resampler will be applied to set the sample rate to the rate given
 * when creating an encoder or decoder
 */
typedef struct {
    // filter bank delay
    size_t delay;

    // filter passband bandwidth
    float bandwidth;

    // filter sidelobe suppression
    float attenuation;

    // filter bank size
    size_t filter_bank_size;
} quiet_resampler_options;

/* Modulator options
 *
 * This set of options is used only by the encoder
 *
 * The modulator is a combination element which interpolates the encoded signal
 * (rescaling in frequency domain) and then mixes it onto a carrier of a given
 * frequency. Finally, a gain is applied, and an optional DC blocker removes DC
 * components.
 */
typedef struct {
    /* Numerical value for shape of interpolation filter
     *
     * These values correspond to those used by liquid DSP. In particular,
     *
     * 1: Nyquist Kaiser
     *
     * 2: Parks-McClellan
     *
     * 3: Raised Cosine
     *
     * 4: Flipped Exponential (Nyquist)
     *
     * 5: Flipped Hyperbolic Secant (Nyquist)
     *
     * 6: Flipped Arc-Hyperbolic Secant (Nyquist)
     *
     * 7: Root-Nyquist Kaiser (Approximate Optimum)
     *
     * 8: Root-Nyquist Kaiser (True Optimum)
     *
     * 9: Root Raised Cosine
     *
     * 10: Harris-Moerder-3
     *
     * 11: GMSK Transmit
     *
     * 12: GMSK Receive
     *
     * 13: Flipped Exponential (root-Nyquist)
     *
     * 14: Flipped Hyperbolic Secant (root-Nyquist)
     *
     * 15: Flipped Arc-Hyperbolic Secant (root-Nyquist)
     *
     * All other values invalid
     */
    unsigned int shape;

    // interpolation factor
    unsigned int samples_per_symbol;

    // interpolation filter delay
    unsigned int symbol_delay;

    // interpolation roll-off factor
    float excess_bw;

    // carrier frequency, [0, 2*pi)
    float center_rads;

    // gain, [0, 0.5]
    float gain;

    // dc blocker options
    quiet_dc_filter_options dc_filter_opt;
} quiet_modulator_options;

/* Demodulator options
 *
 * This set of options is used only by the decoder
 *
 * The demodulator is a combination element which inverts the operations of the
 * modulator. It first mixes down from the carrier and then performs decimation
 * to recover the signal.
 */
typedef struct {
    /* Numerical value for shape of decimation filter
     *
     * This uses the same set of values as quiet_modulator_options.shape
     */
    unsigned int shape;

    // decimation factor
    unsigned int samples_per_symbol;

    // decimation filter delay
    unsigned int symbol_delay;

    // decimation roll-off factor
    float excess_bw;

    // carrier frequency, [0, 2*pi)
    float center_rads;
} quiet_demodulator_options;

/* Encoder options for OFDM
 *
 * These options configure the behavior of OFDM, orthogonal frequency division
 * multiplexing, as used by the encoder. OFDM places the modulated symbols on
 * to multiple orthogonal subcarriers. This can help the decoder estabilish
 * good equalization when used on a system with uneven filtering.
 */
typedef struct {
    // total number of subcarriers used, inlcuding guard bands and pilots
    unsigned int num_subcarriers;

    // number of cyclic prefix samples between symbols
    unsigned int cyclic_prefix_len;

    // number of taper window between symbols
    unsigned int taper_len;

    // number of extra guard subcarriers inserted on left (low freq)
    size_t left_band;

    // number of extra guard subcarriers inserted on right (high freq)
    size_t right_band;
} quiet_ofdm_options;

/* Encoder mode
 *
 * Selects operational mode for encoder/decoder. OFDM and Modem mode
 * use the same modulation schemes while gmsk ignores the supplied
 * scheme and uses its own
 */
typedef enum encodings {
    // Encode/decode in OFDM mode
    ofdm_encoding,

    // Encode/decode in modem mode
    modem_encoding,

    /*
     * Encode/decode in gaussian minimum shift keying mode
     *
     * GMSK mode does not offer the modulation modes given by the other
     * encodings. It has a fairly limited bitrate, but the advantage of
     * GMSK is that its receiver does not need to compute any FFTs, making
     * it suitable for low-power receivers or situations with little
     * computational capacity.
     */
    gmsk_encoding,
} encoding_t;

/* Encoder options
 *
 * This specifies a complete set of options for the encoder in libquiet.
 */
typedef struct {
    // OFDM options, used only by OFDM mode
    quiet_ofdm_options ofdmopt;

    // Interpolation filter and carrier frequency options
    quiet_modulator_options modopt;

    // Resampler configuration (if specified frequency is not 44.1kHz)
    quiet_resampler_options resampler;

    // Encoder mode, one of {ofdm_encoding, modem_encoding, gmsk_encoding}
    encoding_t encoding;

    /* Numerical value for checksum scheme used to confirm integrity of frame
     *
     * These values correspond to those used by liquid DSP. In particular,
     *
     * 1: no error-detection
     *
     * 2: 8-bit checksum
     *
     * 3: 8-bit CRC
     *
     * 4: 16-bit CRC
     *
     * 5: 24-bit CRC
     *
     * 6: 32-bit CRC
     *
     * All other values invalid
     */
    unsigned int checksum_scheme;

    /* Numerical value for inner FEC (forward error correction) scheme
     *
     * These values correspond to those used by liquid DSP. In particular,
     *
     * 1: no error-correction
     *
     * 2: simple repeat code, r1/3
     *
     * 3: simple repeat code, r1/5
     *
     * 4: Hamming (7,4) block code, r1/2 (really 4/7)
     *
     * 5: Hamming (7,4) with extra parity bit, r1/2
     *
     * 6: Hamming (12,8) block code, r2/3
     *
     * 7: Golay (24,12) block code, r1/2
     *
     * 8: SEC-DED (22,16) block code, r8/11
     *
     * 9: SEC-DED (39,32) block code
     *
     * 10: SEC-DED (72,64) block code, r8/9
     *
     * 11: convolutional code r1/2, K=7, dfree=10
     *
     * 12: convolutional code r1/2, K=9, dfree=12
     *
     * 13; convolutional code r1/3, K=9, dfree=18
     *
     * 14: convolutional code 1/6, K=15, dfree<=57 (Heller 1968)
     *
     * 15: perforated convolutional code r2/3, K=7, dfree=6
     *
     * 16: perforated convolutional code r3/4, K=7, dfree=5
     *
     * 17: perforated convolutional code r4/5, K=7, dfree=4
     *
     * 18: perforated convolutional code r5/6, K=7, dfree=4
     *
     * 19: perforated convolutional code r6/7, K=7, dfree=3
     *
     * 20: perforated convolutional code r7/8, K=7, dfree=3
     *
     * 21: perforated convolutional code r2/3, K=9, dfree=7
     *
     * 22: perforated convolutional code r3/4, K=9, dfree=6
     *
     * 23: perforated convolutional code r4/5, K=9, dfree=5
     *
     * 24: perforated convolutional code r5/6, K=9, dfree=5
     *
     * 25: perforated convolutional code r6/7, K=9, dfree=4
     *
     * 26: perforated convolutional code r7/8, K=9, dfree=4
     *
     * 27: Reed-Solomon m=8, n=255, k=223
     *
     * All other values invalid
     */
    unsigned int inner_fec_scheme;

    /* Numerical value for outer FEC (forward error correction) scheme
     *
     * This uses the same set of values as inner_fec_scheme
     */
    unsigned int outer_fec_scheme;

    /* Numerical value for modulation scheme.
     *
     * This value is ignored in GMSK mode
     *
     * These values correspond to those used by liquid DSP. In particular,
     *
     * 1: phase-shift keying-2
     *
     * 2: phase-shift keying-4
     *
     * 3: phase-shift keying-8
     *
     * 4: phase-shift keying-16
     *
     * 5: phase-shift keying-32
     *
     * 6: phase-shift keying-64
     *
     * 7: phase-shift keying-128
     *
     * 8: phase-shift keying-256
     *
     * 9: differential phase-shift keying-2
     *
     * 10: differential phase-shift keying-4
     *
     * 11: differential phase-shift keying-8
     *
     * 12: differential phase-shift keying-16
     *
     * 13: differential phase-shift keying-32
     *
     * 14: differential phase-shift keying-64
     *
     * 15: differential phase-shift keying-128
     *
     * 16: differential phase-shift keying-256
     *
     * 17: amplitude-shift keying-2
     *
     * 18: amplitude-shift keying-4
     *
     * 19: amplitude-shift keying-8
     *
     * 20: amplitude-shift keying-16
     *
     * 21: amplitude-shift keying-32
     *
     * 22: amplitude-shift keying-64
     *
     * 23: amplitude-shift keying-128
     *
     * 24: amplitude-shift keying-256
     *
     * 25: quadrature amplitude-shift keying-4
     *
     * 26: quadrature amplitude-shift keying-8
     *
     * 27: quadrature amplitude-shift keying-16
     *
     * 28: quadrature amplitude-shift keying-32
     *
     * 29: quadrature amplitude-shift keying-64
     *
     * 30: quadrature amplitude-shift keying-128
     *
     * 31: quadrature amplitude-shift keying-256
     *
     * 32: quadrature amplitude-shift keying-512
     *
     * 33: quadrature amplitude-shift keying-1024
     *
     * 34: quadrature amplitude-shift keying-2048
     *
     * 35: quadrature amplitude-shift keying-4096
     *
     * 36: quadrature amplitude-shift keying-8192
     *
     * 37: quadrature amplitude-shift keying-16384
     *
     * 38: quadrature amplitude-shift keying-32768
     *
     * 39: quadrature amplitude-shift keying-65536
     *
     * 40: amplitude phase-shift keying-4
     *
     * 41: amplitude phase-shift keying-8
     *
     * 42: amplitude phase-shift keying-16
     *
     * 43: amplitude phase-shift keying-32
     *
     * 44: amplitude phase-shift keying-64
     *
     * 45: amplitude phase-shift keying-128
     *
     * 46: amplitude phase-shift keying-256
     *
     * 47: binary phase-shift keying
     *
     * 48: quaternary phase-shift keying
     *
     * 49: on-off keying
     *
     * 50: square quadrature amplitude-shift keying-32
     *
     * 51: square quadrature amplitude-shift keying-128
     *
     * 52: V.29 star constellation
     *
     * 53: optimal quadrature amplitude-shift keying-16
     *
     * 54: optimal quadrature amplitude-shift keying-32
     *
     * 55: optimal quadrature amplitude-shift keying-64
     *
     * 56: optimal quadrature amplitude-shift keying-128
     *
     * 57: optimal quadrature amplitude-shift keying-256
     *
     * 58: Virginia Tech logo constellation
     *
     * All other values invalid
     */
    unsigned int mod_scheme;

    /* Maximum frame length
     *
     * This value controls the maximum length of the user-controlled
     * section of the frame. There is overhead in starting new frames,
     * and each frame performs its own CRC check which either accepts or
     * rejects the frame. A frame begins with a synchronization section
     * which the decoder uses to detect and lock on to the frame. Over time,
     * the synchronization will drift, which makes shorter frames easier to
     * decode than longer frames.
     */
    size_t frame_len;
} quiet_encoder_options;

/* Decoder options
 *
 * This specifies a complete set of options for the decoder in libquiet.
 *
 * In order for a decoder to decode the signals from an encoder, certain
 * options must match between both. In particular, the encoding mode and
 * modopt/demodopt must match. Additionally, if ofdm_encoding is used,
 * then the ofdmopt must also match.
 */
typedef struct {
    // OFDM options, used only by OFDM mode
    quiet_ofdm_options ofdmopt;

    // Decimation filter and carrier frequency options
    quiet_demodulator_options demodopt;

    // Resampler configuration (if specified frequency is not 44.1kHz)
    quiet_resampler_options resampler;

    // Encoder mode, one of {ofdm_encoding, modem_encoding, gmsk_encoding}
    encoding_t encoding;

    /*
     * Enable debug mode on receiver
     *
     * In order for this flag to work, libquiet must be compiled in debug mode
     * (#define QUIET_DEBUG 1). Once enabled, this mode causes the decoder to
     * use liquid to create debug files which can be viewed in matlab/octave.
     * These files have the filename format framesync_d, where d is an
     * increasing number. These files can be useful for tracking the decoder's
     * behavior.
     */
    bool is_debug;
} quiet_decoder_options;

typedef struct {
    float real;
    float imaginary;
} quiet_complex;

/* Decoder frame stats
 *
 * This contains information about the decoding process related to a
 * single frame. The frame may have been detected but failed to
 * pass checksum or may have been successfully received.
 */
typedef struct {
    // Raw symbols, in complex plane, as seen after decimation and downmixing
    const quiet_complex *symbols;
    size_t num_symbols;

    // Magnitude of vector from received symbols to reference symbols, in dB
    float error_vector_magnitude;

    // Power level of received signal after decimation and downmixing, in dB
    float received_signal_strength_indicator;

    bool checksum_passed;
} quiet_decoder_frame_stats;

/* cldoc:end-category() */
/* cldoc:begin-category(profile) */

/* Get decoder profile from file
 * @f file pointer which contains a valid JSON libquiet profile set
 * @profilename the string key of the profile to fetch
 *
 * libquiet's configuration options are fairly numerous, and testing can be
 * frustrating when configuration requires recompilation. For this reason,
 * libquiet provides a JSON file containing multiple sets of configuration
 * -- profiles -- and functions to read and validate them.
 *
 * Each profile provides access to every option contained in
 * quiet_encoder_options/quiet_decoder_options. It is hoped that this will
 * give good default options and provide a starting place for users to
 * tune new profiles.
 *
 * quiet_decoder_profile_file reads the profile given by profilename
 * from the file pointer and returns the corresponding quiet_decoder_options.
 *
 * @return a pointer to an initialized quiet_decoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_decoder_options *quiet_decoder_profile_file(FILE *f,
                                                  const char *profilename);

/* Get decoder profile from filename
 * @fname path to a file which will be opened and read, must contain a
 *  valid JSON liquiet profile set
 * @profilename the string key of the profile to fetch
 *
 * quiet_decoder_profile_filename reads the profile given by profilename
 * from the file located at filename and returns the corresponding
 * quiet_decoder_options.
 *
 * @return a pointer to an initialized quiet_decoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_decoder_options *quiet_decoder_profile_filename(const char *fname,
                                                      const char *profilename);

/* Get decoder profile from string
 * @input a string containing a valid JSON libquiet profile set
 * @profilename the string key of the profile to fetch
 *
 * quiet_decoder_profile_str reads the profile given by profilename from
 * the input and returns the corresponding quiet_decoder_options.
 *
 * @return a pointer to an initialized quiet_decoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_decoder_options *quiet_decoder_profile_str(const char *input,
                                                 const char *profilename);

/* Get encoder profile from file
 * @f file pointer which contains a valid JSON libquiet profile set
 * @profilename the string key of the profile to fetch
 *
 * quiet_encoder_profile_file reads the profile given by profilename
 * from the file pointer and returns the corresponding quiet_encoder_options.
 *
 * @return a pointer to an initialized quiet_encoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_encoder_options *quiet_encoder_profile_file(FILE *f,
                                                  const char *profilename);

/* Get encoder profile from filename
 * @fname path to a file which will be opened and read, must contain a
 *  valid JSON liquiet profile set
 * @profilename the string key of the profile to fetch
 *
 * quiet_encoder_profile_filename reads the profile given by profilename
 * from the file located at filename and returns the corresponding
 * quiet_encoder_options.
 *
 * @return a pointer to an initialized quiet_encoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_encoder_options *quiet_encoder_profile_filename(const char *fname,
                                                      const char *profilename);

/* Get encoder profile from string
 * @input a string containing a valid JSON libquiet profile set
 * @profilename the string key of the profile to fetch
 *
 * quiet_encoder_profile_str reads the profile given by profilename from
 * the input and returns the corresponding quiet_encoder_options.
 *
 * @return a pointer to an initialized quiet_encoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_encoder_options *quiet_encoder_profile_str(const char *input,
                                                 const char *profilename);

/* Get list of profile keys from file
 * @f file pointer which contains a valid JSON file
 * @numkeys return value for number of keys found
 *
 * quiet_profile_keys_file reads the JSON file and fetches the keys from the
 * top-level dictionary. It does not perform validation on the profiles
 * themselves, which could be invalid.
 *
 * @return an array of strings with key names or NULL if JSON parsing failed.
 *  must be freed by caller (with free()). the length of this array will be
 *  written to numkeys.
 */
char **quiet_profile_keys_file(FILE *f, size_t *numkeys);

/* Get list of profile keys from filename
 * @fname path to a file which will be opened and read, must contain a
 *  valid JSON file
 * @numkeys return value for number of keys found
 *
 * quiet_profile_keys_filename reads the JSON file found at fname and fetches
 * the keys from the top-level dictionary. It does not perform validation on
 * the profiles themselves, which could be invalid.
 *
 * @return an array of strings with key names or NULL if JSON parsing failed.
 *  must be freed by caller (with free()). the length of this array will be
 *  written to numkeys.
 */
char **quiet_profile_keys_filename(const char *fname, size_t *numkeys);

/* Get list of profile keys from string
 * @input a string containing a valid JSON file
 * @numkeys return value for number of keys found
 *
 * quiet_profile_keys_str reads the JSON in input and fetches the keys from the
 * top-level dictionary. It does not perform validation on the profiles
 * themselves, which could be invalid.
 *
 * @return an array of strings with key names or NULL if JSON parsing failed.
 *  must be freed by caller (with free()). the length of this array will be
 *  written to numkeys.
 */
char **quiet_profile_keys_str(const char *input, size_t *numkeys);

/* cldoc:end-category() */

// Sound encoder
struct quiet_encoder;
typedef struct quiet_encoder quiet_encoder;

/* Create encoder
 * @opt quiet_encoder_options containing encoder configuration
 * @sample_rate Sample rate that encoder will generate at
 *
 * quiet_encoder_create creates and initializes a new libquiet encoder for a
 * given set of options and sample rate. As libquiet makes use of its own
 * resampler, it is suggested to use the default sample rate of your device,
 * so as to not invoke any implicit resamplers.
 *
 * @return pointer to a new encoder object, or NULL if creation failed
 */
quiet_encoder *quiet_encoder_create(const quiet_encoder_options *opt, float sample_rate);

/* Send a single frame
 * @e encoder object
 * @buf user buffer containing the frame payload
 * @len the number of bytes in buf
 *
 * quiet_encoder_send copies the frame provided by the user to an internal
 * transmit queue. By default, this is a nonblocking call and will fail if
 * the queue is full. However, if quiet_encoder_set_blocking has been
 * called first, then it will wait for as much as the timeout length
 * specified there if the frame cannot be immediately written.
 *
 * The frame provided must be no longer than the maximum frame length of the
 * encoder. If the frame is longer, it will be rejected entirely, and no data
 * will be transmitted.
 *
 * If libquiet was built and linked with pthread, then this function may be
 * called from any thread, and by multiple threads concurrently.
 *
 * quiet_encoder_send will return 0 if the queue is closed to signal EOF.
 *
 * quiet_encoder_send will return a negative value and set the last error
 * to quiet_timedout if the send queue is full and no space was made before
 * the timeout
 *
 * quiet_encoder_send will return a negative value and set the last error
 * to quiet_would_block if the send queue is full and the encoder is in
 * nonblocking mode
 *
 * @return the number of bytes copied from the buffer, 0 if the queue
 * is closed, or -1 if sending failed
 */
ssize_t quiet_encoder_send(quiet_encoder *e, const void *buf, size_t len);

/* Set blocking mode of quiet_encoder_send
 * @e encoder object
 * @sec time_t number of seconds to block for
 * @nano long number of nanoseconds to block for
 *
 * quiet_encoder_set_blocking changes the behavior of quiet_encoder_send so
 * that it will block until a frame can be written. It will block for
 * approximately (nano + 1000000000*sec) nanoseconds.
 *
 * If `sec` and `nano` are both 0, then quiet_encoder_send will block
 * indefinitely until a frame is sent.
 *
 * This function is only supported on systems with pthread. Calling
 * quiet_encoder_set_blocking on a host without pthread will assert
 * false.
 *
 */
void quiet_encoder_set_blocking(quiet_encoder *e, time_t sec, long nano);

/* Set nonblocking mode of quiet_encoder_send
 * @e encoder object
 *
 * quiet_encoder_set_nonblocking changes the behavior of quiet_encoder_send
 * so that it will not block if it cannot write a frame. This function
 * restores the default behavior after quiet_encoder_set_blocking has
 * been called.
 *
 */
void quiet_encoder_set_nonblocking(quiet_encoder *e);

/* Clamp frame length to largest possible for sample length
 * @e encoder object
 * @sample_len size of sample block
 *
 * quiet_encoder_clamp_frame_len enables a mode in the encoder which prevents
 * data frames from overlapping multiple blocks of samples, e.g. multiple calls
 * to quiet_encoder_emit. This can be very convenient if your environment
 * cannot keep up in realtime due to e.g. GC pauses. The transmission of data
 * will succeed as long as the blocks of samples are played out smoothly (gaps
 * between blocks are ok, gaps within blocks are not ok).
 *
 * Calling this with the size of your sample block will clamp the frame length
 * of this encoder and toggle the `is_close_frame` flag which will ensure that
 * sample blocks will always end in silence. This will never result in a frame
 * length longer than the one provided in the creation of the encoder, but it
 * may result in a shorter frame length.
 *
 * @return the new frame length
 */
size_t quiet_encoder_clamp_frame_len(quiet_encoder *e, size_t sample_len);

/* Retrieve encoder frame length
 * @e encoder object
 *
 * @return encoder's maximum frame length, e.g. the largest length that can
 *  be passed to quiet_encoder_send
 */
size_t quiet_encoder_get_frame_len(const quiet_encoder *e);

/* Emit samples
 * @e encoder object
 * @samplebuf user-provided array where samples will be written
 * @samplebuf_len length of user-provided array
 *
 * quiet_encoder_emit fills a block of samples pointed to by samplebuf by
 * reading frames from its transmit queue and encoding them into sound by using
 * the configuration specified at creation. These samples can be written out
 * directly to a file or soundcard.
 *
 * If you are using a soundcard, you will have to carefully choose the sample
 * size block. Typically, the largest size is 16384 samples. Larger block sizes
 * will help hide uneven latencies in the encoding process and ensure smoother
 * transmission at the cost of longer latencies.
 *
 * quiet_encoder_emit may return fewer than the number of samples requested.
 * Unlike quiet_encoder_send, quiet_encoder_emit does not block, even when
 * blocking mode is enabled. This is because soundcard interfaces typically
 * require realtime sample generation.
 *
 * If quiet_encoder_emit returns 0, then the transmit queue is closed and
 * empty, and no future calls to quiet_encoder_emit will retrieve any more
 * samples.
 *
 * @return the number of samples written to samplebuf, which shall never
 *  exceed samplebuf_len. If the returned number of samples written is less
 *  than samplebuf_len, then the encoder has finished encoding the payload
 *  (its transmit queue is empty and all state has been flushed out). The user
 *  should 0-fill any remaining length if the block is to be transmitted.
 *
 *  If quiet_encoder_emit returns a negative length, then it will set the
 *  quiet error. Most commonly, this will happen when the transmit queue
 *  is empty and there are no frames ready to send, but the queue is still
 *  open. If and only if the queue is *closed* and has been completely read,
 *  quiet_encoder_emit will return 0 to signal EOF.
 */
ssize_t quiet_encoder_emit(quiet_encoder *e, quiet_sample_t *samplebuf, size_t samplebuf_len);

/* Close encoder
 * @e encoder object
 *
 * quiet_encoder_close closes the encoder object. This has the effect of
 * rejecting any future calls to quiet_encoder_send. Any previously queued
 * frames will be written by quiet_encoder_emit. Once the send queue is empty,
 * quiet_encoder_emit will set last error to quiet_closed.
 *
 */
void quiet_encoder_close(quiet_encoder *e);

/* Destroy encoder
 * @e encoder object
 *
 * quiet_encoder_destroy releases all resources allocated by the quiet_encoder.
 * After calling this function, the user should not call any other encoder
 * functions on the quiet_encoder.
 */
void quiet_encoder_destroy(quiet_encoder *e);

// Sound decoder
struct quiet_decoder;
typedef struct quiet_decoder quiet_decoder;

/* Create decoder
 * @opt quiet_decoder_options containing decoder configuration
 * @sample_rate Sample rate that decoder will consume at
 *
 * quiet_decoder_create creates and initializes a new libquiet decoder for a
 * given set of options and sample rate.
 *
 * It is recommended to use the default sample rate of your device in order
 * to avoid any possible implicit resampling, which can distort samples.
 *
 * @return pointer to new decoder object, or NULL if creation failed.
 */
quiet_decoder *quiet_decoder_create(const quiet_decoder_options *opt, float sample_rate);

/* Try to receive a single frame
 * @d decoder object
 * @data user buffer which quiet will write received frame into
 * @len length of user-supplied buffer
 *
 * quiet_decoder_recv reads one frame from the decoder's receive buffer. By
 * default, this is a nonblocking call and will fail quickly if no frames are
 * ready to be received. However, if quiet_decoder_set_blocking is called
 * prior to this call, then it will wait for as much as the timeout specified
 * there until it can read a frame.
 *
 * If the user's supplied buffer is smaller than the length of the received
 * frame, then only `len` bytes will be copied to `data`. The remaining bytes
 * will be discarded.
 *
 * This function will never return frames for which the checksum has failed.
 *
 * If libquiet was built and linked with pthread, then this function may be
 * called from any thread, and by multiple threads concurrently.
 *
 * quiet_decoder_recv will return 0 if the decoder has been closed and the
 * receive queue is empty.
 *
 * quiet_decoder_recv will return a negative value and set the last error
 * to quiet_timedout if blocking mode is enabled and no frame could be read
 * before the timeout.
 *
 * quiet_decoder_recv will return a negative value and set the last error
 * to quiet_would_block if nonblocking mode is enabled and no frame was
 * available.
 *
 * @return number of bytes written to buffer, 0 at EOF, or -1 if no frames
 * available
 */
ssize_t quiet_decoder_recv(quiet_decoder *d, uint8_t *data, size_t len);

/* Set blocking mode of quiet_decoder_recv
 * @d decoder object
 * @sec time_t number of seconds to block for
 * @nano long number of nanoseconds to block for
 *
 * quiet_decoder_set_blocking changes the behavior of quiet_decoder_recv so
 * that it will block until a frame can be read. It will block for
 * approximately (nano + 1000000000*sec) nanoseconds.
 *
 * If `sec` and `nano` are both 0, then quiet_decoder_recv will block
 * indefinitely until a frame is received.
 *
 * This function is only supported on systems with pthread. Calling
 * quiet_decoder_set_blocking on a host without pthread will assert
 * false.
 *
 */
void quiet_decoder_set_blocking(quiet_decoder *d, time_t sec, long nano);

/* Set nonblocking mode of quiet_decoder_recv
 * @d decoder object
 *
 * quiet_decoder_set_nonblocking changes the behavior of quiet_decoder_recv
 * so that it will not block if it cannot read a frame. This function
 * restores the default behavior after quiet_decoder_set_blocking has
 * been called.
 *
 */
void quiet_decoder_set_nonblocking(quiet_decoder *d);

/* Feed received sound samples to decoder
 * @d decoder object
 * @samplebuf array of samples received from sound card
 * @sample_len number of samples in samplebuf
 *
 * quiet_decoder_consume consumes sound samples and decodes them to frames.
 * These can be samples obtained directly from a sound file, a soundcard's
 * microphone, or any other source which can receive quiet_sample_t (float).
 *
 * If you are using a soundcard, it is recommended to use the largest block
 * size offered. Typically, this is 16384 samples. Larger block sizes will
 * help hide uneven latencies in the decoding process and ensure smoother
 * reception at the cost of longer latencies.
 *
 */
void quiet_decoder_consume(quiet_decoder *d, const quiet_sample_t *samplebuf, size_t sample_len);

/* Check if a frame is likely being received
 * @d decoder object
 *
 * quiet_decoder_frame_in_progress determines if a frame is likely in the
 * process of being received. It inspects information in the decoding process
 * and will be relevant to the last call to quiet_decoder_consume. There
 * is no guarantee of accuracy from this function, and both false-negatives
 * and false-positives can occur.
 *
 * The output of this function can be useful to avoid collisions when two
 * pairs of encoders/decoders share the same channel, e.g. in half-duplex.
 *
 * This function must be called from the same thread which calls
 * quiet_decoder_consume.
 *
 * @return true if a frame is likely being received
 */
bool quiet_decoder_frame_in_progress(quiet_decoder *d);

/* Flush existing state through decoder
 * @d decoder object
 *
 * quiet_decoder_flush empties out all internal buffers and attempts to decode
 * them
 *
 * This function need only be called after the sound stream has stopped.
 * It is especially useful for reading from sound files where there are no
 * trailing samples to "push" the decoded data through the decoder's filters
 */
void quiet_decoder_flush(quiet_decoder *d);

/* Close decoder
 * @d decoder object
 *
 * quiet_decoder_close closes the decoder object. Future calls to
 * quiet_decoder_consume will still attempt the decoding process but
 * will not enqueue any decoded frames into the receive queue, e.g.
 * they become cpu-expensive no-ops. Any previously enqueued frames
 * can still be read out by quiet_decoder_recv, and once the receive queue
 * is empty, quiet_decoder_recv will set the last error to quiet_closed.
 *
 */
void quiet_decoder_close(quiet_decoder *d);

/* Return number of failed frames
 * @d decoder object
 *
 * quiet_decoder_checksum_fails returns the total number of frames decoded
 * but which failed checksum across the lifetime of the decoder.
 *
 * @return Total number of frames received with failed checksums
 */
unsigned int quiet_decoder_checksum_fails(const quiet_decoder *d);

/* Fetch stats from last call to quiet_decoder_consume
 * @d decoder object
 * @num_frames number of frames at returned pointer
 *
 * quiet_decoder_consume_stats returns detailed info about the decoding
 * process from the last call to quiet_decoder_consume_stats. It will
 * save information on up to 8 frames. This includes frames which failed
 * checksum. If quiet_decoder_consume found more than 8 frames, then
 * information on only the first 8 frames will be saved.
 *
 * In order to use this functionality, quiet_decoder_enable_stats
 * must be called on the decoder object before calling
 * quiet_decoder_consume.
 *
 * This function must be called from the same thread that calls
 * quiet_decoder_consume.
 *
 * @return quiet_decoder_frame_stats which is an array of structs containing
 *  stats info, num_frames long
 */
const quiet_decoder_frame_stats *quiet_decoder_consume_stats(quiet_decoder *d, size_t *num_frames);

const quiet_decoder_frame_stats *quiet_decoder_recv_stats(quiet_decoder *d);

/* Enable stats collection
 * @d decoder object
 *
 * quiet_decoder_enable_stats allocates the required memory needed to save
 * symbol and other information on each frame decode. Italso adds a small
 * overhead needed to copy this information into an internal buffer.
 *
 * By default, stats collection is disabled. Therefore, if the user would like
 * to use quiet_decoder_consume_stats, then they must first call
 * quiet_decoder_enable_stats.
 */
void quiet_decoder_enable_stats(quiet_decoder *d);

/* Disable stats collection
 * @d decoder object
 *
 * quiet_decoder_disable_stats frees all memory associated with
 * stats collection.
 */
void quiet_decoder_disable_stats(quiet_decoder *d);

void quiet_decoder_set_stats_blocking(quiet_decoder *d, time_t sec, long nano);

void quiet_decoder_set_stats_nonblocking(quiet_decoder *d);
/* Destroy decoder
 * @d decoder object
 *
 * quiet_decoder_destroy releases all resources allocated by the quiet_decoder.
 * After calling this function, the user should not call any other decoder
 * functions on the decoder.
 */
void quiet_decoder_destroy(quiet_decoder *d);

#ifdef __cplusplus
}
#endif

#endif
