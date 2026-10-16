; FamiTone2 data for neslib's music_init() / sounds_init(). Regenerate with tools/export_audio.sh.
.section .rodata.music_data,"a",@progbits
.globl music_data
music_data:
  .include "audio/chill.s"

.section .rodata.sounds_data,"a",@progbits
.globl sounds_data
sounds_data:
  .include "audio/sfx.s"
