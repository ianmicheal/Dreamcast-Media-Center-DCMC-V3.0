#ifndef FIFO_H
#define FIFO_H

#include <kos/mutex.h>

#define FIFO_STAT_NULL 0x00
#define FIFO_STAT_INIT 0x01
#define FIFO_STAT_ON   0x02
#define FIFO_STAT_OFF  0x03
#define FIFO_STAT_ERR  0x04
#define FIFO_STAT_DONE 0x05

mutex_t * fifo_muta, *fifo_mutv;
#define FIFO_create_audio_mutex()  { fifo_muta = mutex_create(); }
#define FIFO_lock_audio_mutex()    { mutex_lock( fifo_muta );    }
#define FIFO_unlock_audio_mutex()  { mutex_unlock( fifo_muta );  }
#define FIFO_destroy_audio_mutex() { mutex_destroy( fifo_muta ); }
#define FIFO_create_video_mutex()  { fifo_mutv = mutex_create(); }
#define FIFO_lock_video_mutex()    { mutex_lock( fifo_mutv );    }
#define FIFO_unlock_video_mutex()  { mutex_unlock( fifo_mutv );  }
#define FIFO_destroy_video_mutex() { mutex_destroy( fifo_mutv ); }

typedef struct
{
    int size;            // Size of Buffer
    int frames;          // Number of Frames(Packets) stored in Buffer
    int fsize[1024];     // Frame Size Array
    unsigned char *buf;  // Buffer for encoded Packets  
} FifoBuffer;

#endif
