#define MAGIC 'X'
#define CMD_1  _IO(MAGIC,0)
#define CMD_GET  _IOW(MAGIC,1,int)
#define CMD_D   _IO(MAGIC,2)

