 PCM to G711 Fast Conversions
==============================

# Build example:

        make

# Conversions

        ./example example_files/alaw.raw alaw_pcm ulaw_to_pcm.audio
        ./example example_files/ulaw.raw ulaw_pcm alaw_to_pcm.audio
        ./example example_files/pcm.raw pcm_alaw pcm_to_alaw.audio
        ./example example_files/pcm.raw pcm_ulaw pcm_to_ulaw.audio

# To play files:

        brew install sox

        play -t s16 -r 44100 -c 2 -e signed-integer example_files/pcm.raw
        play -t raw -r 44100 -c 2 -e a-law example_files/alaw.raw
        play -t raw -r 44100 -c 2 -e u-law example_files/ulaw.raw

        play -t s16 -r 44100 -c 2 -e signed-integer alaw_to_pcm.audio
        play -t s16 -r 44100 -c 2 -e signed-integer ulaw_to_pcm.audio
        play -t raw -r 44100 -c 2 -e a-law          pcm_to_alaw.audio
        play -t raw -r 44100 -c 2 -e u-law          pcm_to_ulaw.audio
