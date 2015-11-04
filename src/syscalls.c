#include <sys/stat.h>
#include <errno.h>

extern int errno;


int _write(int fd, const char *buf, size_t cnt)
{
  int i;

  for (i = 0; i < cnt; i++)
    //    putch(buf[i]);

  return cnt;
}


int _getpid(int file)
{
	return 1;
}

int _kill(int file)
{
	return 1;
}


int _close(int file) {
	return 0;
}

int _fstat(int file, struct stat *st) {
	return 0;
}

int _isatty(int file) {
	return 1;
}

int _lseek(int file, int ptr, int dir) {
	return 0;
}

int _open(const char *name, int flags, int mode) {
	return -1;
}

int _read(int file, char *ptr, int len) {
	return 0;
}

/*int crashfun(void){
  static int inc=0;
  return inc++;
  }*/

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");

caddr_t _sbrk (struct _reent *r, int incr) // was _sbrk_r
{
	extern char   end asm ("end"); // Defined by the linker. 
	static char * heap_end;
	char *        prev_heap_end;


	if (heap_end == NULL)
		heap_end = & end;

	prev_heap_end = heap_end;

	if (heap_end + incr > stack_ptr)
	{

	  /*#if 0
		extern void abort (void);

		_write (1, "_sbrk: Heap and stack collision\n", 32);

		abort ();
		#else*/
		errno = ENOMEM;
		
		return (caddr_t) -1;
		//#endif
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}
