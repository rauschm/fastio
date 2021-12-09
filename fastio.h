#ifndef _INC_FASTIO_H
#define _INC_FASTIO_H

//------------------------------------------------------------------------------
//  Include-Dateien
//------------------------------------------------------------------------------
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#pragma comment(lib, "mincore")
#else
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
#ifdef __cplusplus
#define c_only(c)
#else
#define c_only(c) c
#endif

#ifndef _WIN32
#define Perror perror
#endif

#ifdef _MSC_VER
#define inline __forceinline
#else
#ifdef __GNUC__
#define inline inline __attribute__((always_inline))
#else
/* xlc auf AIX */
#define inline __inline__ __attribute__((always_inline))
#endif
#endif

#ifdef _WIN32
#define MemoryMapTwice(LOC,SIZ,RET1,RET2) \
     (LOC = (char*) VirtualAlloc2(NULL, NULL, SIZ * 2, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, NULL, 0)) == NULL \
  || VirtualFree(LOC, SIZ, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER) == 0 \
  || (fh = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SIZ, NULL)) == NULL \
  || MapViewOfFile3(fh, NULL, RET1 LOC,       0, SIZ, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, NULL, 0) == NULL \
  || MapViewOfFile3(fh, NULL, RET2 LOC + SIZ, 0, SIZ, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, NULL, 0) == NULL \
  || CloseHandle(fh) == 0
#define MemoryUnmapBoth(LOC,SIZ) { UnmapViewOfFile(LOC); UnmapViewOfFile(LOC + SIZ); }
#else
#ifdef __linux__
#define MemoryMapTwice(LOC,SIZ,RET1,RET2) \
     (LOC = (char*) valloc(SIZ * 2)) == NULL \
  || (fd = shmget(IPC_PRIVATE, SIZ, S_IRUSR | S_IWUSR)) == -1 \
  || shmat(fd, RET1 LOC,       SHM_REMAP) != LOC \
  || shmat(fd, RET2 LOC + SIZ, SHM_REMAP) != LOC + SIZ \
  || shmctl(fd, IPC_RMID, 0) == -1
#define MemoryUnmapBoth(LOC,SIZ) { shmdt(LOC); shmdt(LOC + SIZ); }
#else
#define MemoryMapTwice(LOC,SIZ,RET1,RET2) \
     (LOC = (char*) mmap(0, SIZ * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED \
  || (fd = shm_open("**fastio**", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1 \
  || shm_unlink("**fastio**") == -1 \
  || ftruncate(fd, SIZ * 2) == -1 \
  || mmap(RET1 LOC,       SIZ, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED \
  || mmap(RET2 LOC + SIZ, SIZ, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED \
  || close(fd) == -1
#define MemoryUnmapBoth(LOC,SIZ) { munmap(LOC, SIZ);  munmap(LOC + SIZ, SIZ); }
#endif
#endif

#ifndef __cplusplus
#define __NARG__(...) __NARG_I__((__VA_ARGS__,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49, \
                                              48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33, \
                                              32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17, \
                                              16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
#define __NARG_I__(X) __NARG_F__ X
#define __NARG_F__(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, \
                   _17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32, \
                   _33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48, \
                   _49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,N,...) N

#define __VFUNC__(func, ...) __VFUNC_I__(func, __NARG__(__VA_ARGS__))(__VA_ARGS__)
#define __VFUNC_I__(name, n) __VFUNC_J__(name, n)
#define __VFUNC_J__(name, n) __VFUNC_F__(name, n)
#define __VFUNC_F__(name, n) name##n
#endif

//------------------------------------------------------------------------------
//  fehlende Funktionen
//------------------------------------------------------------------------------
#ifdef _WIN32
static void Perror(char const* s) {
  UINT  acp = GetACP();
  UINT  ocp = GetConsoleOutputCP();
  DWORD err = GetLastError();
  char* buf;
  char  buf_spare[401];
  DWORD len;

  len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buf, 0, NULL);
  if (len == 0) {
    buf = buf_spare;
    sprintf(buf, "error #%d occcured\r\n", err);
  } else {
    if (ocp != acp) {
      WCHAR wbuf_spare[100];
      int wlen = MultiByteToWideChar(acp, 0, buf, len, NULL, 0);
      WCHAR* wbuf = (WCHAR*) LocalAlloc(LMEM_FIXED, wlen * sizeof(WCHAR));
      if (wbuf == NULL) {
        if (len > sizeof(wbuf_spare) / sizeof(WCHAR)) {
          len = sizeof(wbuf_spare) / sizeof(WCHAR);
        }
        wbuf = wbuf_spare;
        wlen = sizeof(wbuf_spare) / sizeof(WCHAR);
      }
      wlen = MultiByteToWideChar(acp, 0, buf, len, wbuf, wlen);
      LocalFree(buf);

      len = WideCharToMultiByte(ocp, 0, wbuf, wlen, NULL, 0, NULL, NULL);
      buf = (char*) LocalAlloc(LMEM_FIXED, len + 1);
      if (buf == NULL) {
        buf = buf_spare;
        len = sizeof(buf_spare) - 1;
      }
      len = WideCharToMultiByte(ocp, 0, wbuf, wlen, buf, len, NULL, NULL);
      buf[len] = '\0';
      if (wbuf != wbuf_spare) {
        LocalFree(wbuf);
      }
    }
  }
  fprintf(stderr, "%s: %s", s, buf);
  if (buf != buf_spare) {
    LocalFree(buf);
  }
}
#endif

#ifndef _GNU_SOURCE
static inline void *memrchr(void const* s, int c, size_t n) {
  char const* p = (char const*) s + n;
  do {
    if (*--p == (char) c) {
      return (void*) p;
    }
  } while (--n != 0);
  return NULL;
}
#endif

//------------------------------------------------------------------------------
//  Typen für Casts zur Auswahl verschiedener Hexadezimal-Formate
//------------------------------------------------------------------------------
#ifdef __cplusplus
namespace fio {
  enum class x     : unsigned int;
  enum class x0x   : unsigned int;
  enum class xpg   : unsigned int;
  enum class llx   : unsigned long long int;
  enum class llx0x : unsigned long long int;
  enum class llxpg : unsigned long long int;
}
#endif

//==============================================================================
//  reader
//==============================================================================
#ifdef __cplusplus
class fio_reader {
private:
#else
typedef struct fio_reader {
#endif

  FILE*  const rb_f;
  size_t const rb_buffer_size;
  size_t const rb_buffer_count;
  size_t       rb_size;
  char*        rb_begin;
  char*        rb_end;
  char*        rb_stop;
  char*        rb_data_end;
  char*        rb_data_stop;
  char*        rb_pos;

#ifndef __cplusplus
} fio_reader;
#endif

  static inline unsigned int fio_r_dtou(char** const p) {
    unsigned int u;
    unsigned int d;
    u = **p - '0';
    if ((d = *++ * p - '0') <= 9) {
      u = u * 10 + d;
      if ((d = *++ * p - '0') <= 9) {
        u = u * 10 + d;
        if ((d = *++ * p - '0') <= 9) {
          u = u * 10 + d;
          if ((d = *++ * p - '0') <= 9) {
            u = u * 10 + d;
            if ((d = *++ * p - '0') <= 9) {
              u = u * 10 + d;
              if ((d = *++ * p - '0') <= 9) {
                u = u * 10 + d;
                if ((d = *++ * p - '0') <= 9) {
                  u = u * 10 + d;
                  if ((d = *++ * p - '0') <= 9) {
                    u = u * 10 + d;
                    if ((d = *++ * p - '0') <= 9) {
                      u = u * 10 + d;
                      *p += 1;
    } } } } } } } } }
    *p += 1;
    return u;
  }

  static inline int fio_r_dtoi(char** const p) {
    int sign;
    if ((sign = (**p != '-') * 2 - 1) < 0) {
      *p += 1;
    }
    return sign * fio_r_dtou(p);
  }

  static inline unsigned int fio_r_xtou(char** const p) {
    unsigned int u;
    unsigned int d;
    u =           (d = *(*p)++ - '0') <= 9 ? d : d - 7;
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    *p += 1;
    return u;
  }

  static inline unsigned long long int fio_r_dtoU(char** const p) {
    unsigned long long int u;
    unsigned int d;
    u = **p - '0';
    if ((d = *++ * p - '0') <= 9) {
      u = u * 10 + d;
      if ((d = *++ * p - '0') <= 9) {
        u = u * 10 + d;
        if ((d = *++ * p - '0') <= 9) {
          u = u * 10 + d;
          if ((d = *++ * p - '0') <= 9) {
            u = u * 10 + d;
            if ((d = *++ * p - '0') <= 9) {
              u = u * 10 + d;
              if ((d = *++ * p - '0') <= 9) {
                u = u * 10 + d;
                if ((d = *++ * p - '0') <= 9) {
                  u = u * 10 + d;
                  if ((d = *++ * p - '0') <= 9) {
                    u = u * 10 + d;
                    if ((d = *++ * p - '0') <= 9) {
                      u = u * 10 + d;
                      if ((d = *++ * p - '0') <= 9) {
                        u = u * 10 + d;
                        if ((d = *++ * p - '0') <= 9) {
                          u = u * 10 + d;
                          if ((d = *++ * p - '0') <= 9) {
                            u = u * 10 + d;
                            if ((d = *++ * p - '0') <= 9) {
                              u = u * 10 + d;
                              if ((d = *++ * p - '0') <= 9) {
                                u = u * 10 + d;
                                if ((d = *++ * p - '0') <= 9) {
                                  u = u * 10 + d;
                                  if ((d = *++ * p - '0') <= 9) {
                                    u = u * 10 + d;
                                    if ((d = *++ * p - '0') <= 9) {
                                      u = u * 10 + d;
                                      if ((d = *++ * p - '0') <= 9) {
                                        u = u * 10 + d;
                                        if ((d = *++ * p - '0') <= 9) {
                                          u = u * 10 + d;
                                          *p += 1;
    } } } } } } } } } } } } } } } } } } }
    *p += 1;
    return u;
  }

  static inline long long int fio_r_dtoI(char** const p) {
    int sign;
    if ((sign = (**p != '-') * 2 - 1) < 0) {
      *p += 1;
    }
    return sign * fio_r_dtoU(p);
  }

  static inline unsigned long long int fio_r_xtoU(char** const p) {
    unsigned long long int u;
    unsigned int d;
    u =           (d = *(*p)++ - '0') <= 9 ? d : d - 7;
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    u = u * 16 + ((d = *(*p)++ - '0') <= 9 ? d : d - 7);
    *p += 1;
    return u;
  }

  static inline char const* fio_r_stoc(char** const p, const char c) {
    char const* r = *p;
    *p = (char*) memchr(r, c, (size_t) -1 / 2);
    *(*p)++ = '\0';
    return r;
  }

  c_only(static) inline void fio_r_init(c_only(fio_reader* this)) {
    char*  rb;
#ifdef _WIN32
    HANDLE fh;
#else
    int    fd;
#endif

    if (this->rb_f == NULL) {
      errno = EINVAL;
      perror("r_init error");
      exit(1);
    }

    // Ringpuffer aufbauen
    this->rb_size = this->rb_buffer_size * this->rb_buffer_count;
    if (MemoryMapTwice(rb, this->rb_size,,this->rb_begin =)) {
      Perror("memory error");
      exit(1);
    }
    // Ringpuffer einrichten
    this->rb_end        = this->rb_begin + this->rb_size;
    this->rb_stop       = this->rb_begin;
    this->rb_data_end   = this->rb_begin;
    this->rb_data_stop  = this->rb_begin;
    this->rb_pos        = this->rb_begin;
    setvbuf(this->rb_f, NULL, _IONBF, 0);
  }

  c_only(static) inline void fio_r_exit(c_only(fio_reader* this)) {
    MemoryUnmapBoth(this->rb_begin - this->rb_size, this->rb_size);
  }

  c_only(static) inline int fio_r(c_only(fio_reader* this)) {
    if (this->rb_pos == this->rb_data_stop) {
      // wenn alle verarbeitbaren eingelesenen Daten verarbeitet wurden
      if (this->rb_data_end < this->rb_stop) {
        // wenn Dateiende
        return 0;
      }
      do {
        if (this->rb_stop == this->rb_end) { // wenn das Ende des hinter(st)en Pufferbereichs erreicht ist
          // kommen weitere Daten in den ersten Pufferbereich
          this->rb_stop = this->rb_begin;
          this->rb_pos -= this->rb_size;
        }
        // weitere Daten einlesen
        this->rb_data_end = this->rb_stop + fread(this->rb_stop, 1, this->rb_buffer_size, this->rb_f);
        this->rb_stop += this->rb_buffer_size;
        if ((this->rb_data_end < this->rb_stop) && ferror(this->rb_f)) {
          perror("read error");
          exit(1);
        }
        if (this->rb_pos == this->rb_data_end) { // wenn Dateiende
          return 0;
        }
        // nach dem letzten Zeilenendezeichen suchen, wiederholen wenn nicht gefunden
      } while ((this->rb_data_stop = (char*) memrchr(this->rb_begin, '\n', this->rb_data_end - this->rb_begin)) == NULL);
      this->rb_data_stop += 1;
    }
    return 1;
  }

#ifdef __cplusplus

  static inline void fio_read_base(int& i, char*& p) {
    i = fio_r_dtoi(&p);
  }
  static inline void fio_read_base_nl(int& i, char*& p) { fio_read_base(i, p); };

  static inline void fio_read_base(unsigned int& u, char*& p) {
    u = fio_r_dtou(&p);
  }
  static inline void fio_read_base_nl(unsigned int& u, char*& p) { fio_read_base(u, p); };

  static inline void fio_read_base(fio::x& h, char*& p) {
    h = (fio::x) fio_r_xtou(&p);
  }
  static inline void fio_read_base_nl(fio::x& h, char*& p) { fio_read_base(h, p); };

  static inline void fio_read_base(long long int& i, char*& p) {
    i = fio_r_dtoI(&p);
  }
  static inline void fio_read_base_nl(long long int& i, char*& p) { fio_read_base(i, p); };

  static inline void fio_read_base(unsigned long long int& u, char*& p) {
    u = fio_r_dtoU(&p);
  }
  static inline void fio_read_base_nl(unsigned long long int& u, char*& p) { fio_read_base(u, p); };

  static inline void fio_read_base(fio::llx& h, char*& p) {
    h = (fio::llx) fio_r_xtoU(&p);
  }
  static inline void fio_read_base_nl(fio::llx& h, char*& p) { fio_read_base(h, p); };

  static inline void fio_read_base(const char*& a, char*& p) {
    a = fio_r_stoc(&p, ';');
  }

  static inline void fio_read_base_nl(const char*& a, char*& p) {
    a = fio_r_stoc(&p, '\n');
  }

public:
  fio_reader(FILE* f = stdin, size_t buffer_size_factor = 4, size_t buffer_count = 2)
    : rb_f(f), rb_buffer_size (65536 << buffer_size_factor),
      rb_buffer_count(buffer_count), rb_size((65536 << buffer_size_factor) * buffer_count) {
    fio_r_init();
  }

  ~fio_reader() {
    fio_r_exit();
  }

  template <typename T>
  void fio_read_loop(T& v) {
    fio_read_base_nl(v, rb_pos);
  }

  template <typename T, typename ... Args>
  void fio_read_loop(T& v, Args& ...args) {
    fio_read_base(v, rb_pos);
    fio_read_loop(args...);
  }

  template <typename ... Args>
  bool fio_read(Args& ...args) {
    if (!fio_r()) {
      return false;
    }
    fio_read_loop(args...);
    return true;
  }
};
#else

#define fio_reader(...) __VFUNC__(fio_reader, __VA_ARGS__)

#define fio_reader1(t)       fio_reader t = {stdin, 65536 << 4, 2}
#define fio_reader2(t,f)     fio_reader t = {f, 65536 << 4, 2}
#define fio_reader3(t,f,s)   fio_reader t = {f, 65536 << s, 2}
#define fio_reader4(t,f,s,c) fio_reader t = {f, 65536 << s, c}

#define fio_init_r(t) fio_r_init(&t)
#define fio_exit_r(t) fio_r_exit(&t)

#define fio_read(...) __VFUNC__(fio_read, __VA_ARGS__)

#define fio_read3(t,f1,v1) \
  (fio_r(&t) ? v1 = fio_r_nl_##f1(&t.rb_pos), 1 : 0)
#define fio_read5(t,f1,v1,f2,v2) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_nl_##f2(&t.rb_pos), 1 : 0)
#define fio_read7(t,f1,v1,f2,v2,f3,v3) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_nl_##f3(&t.rb_pos), 1 : 0)
#define fio_read9(t,f1,v1,f2,v2,f3,v3,f4,v4) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_nl_##f4(&t.rb_pos), 1 : 0)
#define fio_read11(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_dl_##f4(&t.rb_pos), \
               v5 = fio_r_nl_##f5(&t.rb_pos), 1 : 0)
#define fio_read13(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_dl_##f4(&t.rb_pos), \
               v5 = fio_r_dl_##f5(&t.rb_pos), \
               v6 = fio_r_nl_##f6(&t.rb_pos), 1 : 0)
#define fio_read15(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_dl_##f4(&t.rb_pos), \
               v5 = fio_r_dl_##f5(&t.rb_pos), \
               v6 = fio_r_dl_##f6(&t.rb_pos), \
               v7 = fio_r_nl_##f7(&t.rb_pos), 1 : 0)
#define fio_read17(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7,f8,v8) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_dl_##f4(&t.rb_pos), \
               v5 = fio_r_dl_##f5(&t.rb_pos), \
               v6 = fio_r_dl_##f6(&t.rb_pos), \
               v7 = fio_r_dl_##f7(&t.rb_pos), \
               v8 = fio_r_nl_##f8(&t.rb_pos), 1 : 0)
#define fio_read19(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7,f8,v8,f9,v9) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_dl_##f4(&t.rb_pos), \
               v5 = fio_r_dl_##f5(&t.rb_pos), \
               v6 = fio_r_dl_##f6(&t.rb_pos), \
               v7 = fio_r_dl_##f7(&t.rb_pos), \
               v8 = fio_r_dl_##f8(&t.rb_pos), \
               v9 = fio_r_nl_##f9(&t.rb_pos), 1 : 0)
#define fio_read21(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7,f8,v8,f9,v9,fa,va) \
  (fio_r(&t) ? v1 = fio_r_dl_##f1(&t.rb_pos), \
               v2 = fio_r_dl_##f2(&t.rb_pos), \
               v3 = fio_r_dl_##f3(&t.rb_pos), \
               v4 = fio_r_dl_##f4(&t.rb_pos), \
               v5 = fio_r_dl_##f5(&t.rb_pos), \
               v6 = fio_r_dl_##f6(&t.rb_pos), \
               v7 = fio_r_dl_##f7(&t.rb_pos), \
               v8 = fio_r_dl_##f8(&t.rb_pos), \
               v9 = fio_r_dl_##f9(&t.rb_pos), \
               va = fio_r_nl_##fa(&t.rb_pos), 1 : 0)

#define fio_r_dl_dtou fio_r_dtou
#define fio_r_nl_dtou fio_r_dtou
#define fio_r_dl_dtoi fio_r_dtoi
#define fio_r_nl_dtoi fio_r_dtoi
#define fio_r_dl_xtou fio_r_xtou
#define fio_r_nl_xtou fio_r_xtou
#define fio_r_dl_dtoU fio_r_dtoU
#define fio_r_nl_dtoU fio_r_dtoU
#define fio_r_dl_dtoI fio_r_dtoI
#define fio_r_nl_dtoI fio_r_dtoI
#define fio_r_dl_xtoU fio_r_xtoU
#define fio_r_nl_xtoU fio_r_xtoU
#define fio_r_dl_stoc(p) fio_r_stoc(p,';')
#define fio_r_nl_stoc(p) fio_r_stoc(p,'\n')

#define du(x) dtou, x
#define di(x) dtoi, x
#define xu(x) xtou, x
#define dU(x) dtoU, x
#define dI(x) dtoI, x
#define xU(x) xtoU, x
#define sc(x) stoc, x

#endif

//==============================================================================
//  writer
//==============================================================================
#ifdef __cplusplus
class fio_writer {
private:
#else
typedef struct fio_writer {
#endif

  FILE*  const wb_f;
  size_t const wb_buffer_size;
  size_t const wb_buffer_count;
  size_t       wb_size;
  char*        wb_begin;
  char*        wb_end;
  char*        wb_start;
  char*        wb_stop;
  char*        wb_pos;

#ifndef __cplusplus
} fio_writer;
#endif

  static inline void fio_w_utod(unsigned int u, char** const p, char const c) {
    switch (u >= 1000000000 ? 9 :
            u >=  100000000 ? 8 :
            u >=   10000000 ? 7 :
            u >=    1000000 ? 6 :
            u >=     100000 ? 5 :
            u >=      10000 ? 4 :
            u >=       1000 ? 3 :
            u >=        100 ? 2 :
            u >=         10 ? 1 : 0) {
      case 9: *(*p)++ = '0' + u / 1000000000; u %= 1000000000;
      case 8: *(*p)++ = '0' + u /  100000000; u %=  100000000;
      case 7: *(*p)++ = '0' + u /   10000000; u %=   10000000;
      case 6: *(*p)++ = '0' + u /    1000000; u %=    1000000;
      case 5: *(*p)++ = '0' + u /     100000; u %=     100000;
      case 4: *(*p)++ = '0' + u /      10000; u %=      10000;
      case 3: *(*p)++ = '0' + u /       1000; u %=       1000;
      case 2: *(*p)++ = '0' + u /        100; u %=        100;
      case 1: *(*p)++ = '0' + u /         10; u %=         10;
      case 0: *(*p)++ = '0' + u;
    }
    *(*p)++ = c;
  }

  static inline void fio_w_itod(int i, char** const p, char const c) {
    if (i < 0) {
      *(*p)++ = '-';
      i = -i;
    }
    fio_w_utod(i, p, c);
  }

  static inline void fio_w_utox(unsigned int u, char** const p, char const c) {
    unsigned int d;
    *(*p)++ = '0' + ((d =   u >> 28        ) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 24) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 20) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 16) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 12) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >>  8) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >>  4) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d =   u        & 0xF ) <= 9 ? d : d + 7);
    *(*p)++ = c;
  }

  static inline void fio_w_utox_0x(unsigned int u, char** const p, char const c) {
    *(*p)++ = '0';
    *(*p)++ = 'x';
    fio_w_utox(u, p, c);
  }

  static inline void fio_w_utox_pg(unsigned int u, char** const p, char const c) {
    *(*p)++ = '\\';
    *(*p)++ = 'x';
    fio_w_utox(u, p, c);
  }

  static inline void fio_w_Utod(unsigned long long int u, char** const p, char const c) {
    unsigned int u1, u2, u3;
    u1 = (unsigned int) (u / 1000000000000ULL);
    u2 = (unsigned int) (u % 1000000000000ULL / 10000ULL);
    u3 = u % 10000ULL;
    switch (u1 >= 10000000 ? 19 :
            u1 >=  1000000 ? 18 :
            u1 >=   100000 ? 17 :
            u1 >=    10000 ? 16 :
            u1 >=     1000 ? 15 :
            u1 >=      100 ? 14 :
            u1 >=       10 ? 13 :
            u1 >=        1 ? 12 :
            u2 >= 10000000 ? 11 :
            u2 >=  1000000 ? 10 :
            u2 >=   100000 ?  9 :
            u2 >=    10000 ?  8 :
            u2 >=     1000 ?  7 :
            u2 >=      100 ?  6 :
            u2 >=       10 ?  5 :
            u2 >=        1 ?  4 :
            u3 >=     1000 ?  3 :
            u3 >=      100 ?  2 :
            u3 >=       10 ?  1 : 0) {
      case 19: *(*p)++ = '0' + u1 / 10000000; u1 %= 10000000;
      case 18: *(*p)++ = '0' + u1 /  1000000; u1 %=  1000000;
      case 17: *(*p)++ = '0' + u1 /   100000; u1 %=   100000;
      case 16: *(*p)++ = '0' + u1 /    10000; u1 %=    10000;
      case 15: *(*p)++ = '0' + u1 /     1000; u1 %=     1000;
      case 14: *(*p)++ = '0' + u1 /      100; u1 %=      100;
      case 13: *(*p)++ = '0' + u1 /       10; u1 %=       10;
      case 12: *(*p)++ = '0' + u1;
      case 11: *(*p)++ = '0' + u2 / 10000000; u2 %= 10000000;
      case 10: *(*p)++ = '0' + u2 /  1000000; u2 %=  1000000;
      case  9: *(*p)++ = '0' + u2 /   100000; u2 %=   100000;
      case  8: *(*p)++ = '0' + u2 /    10000; u2 %=    10000;
      case  7: *(*p)++ = '0' + u2 /     1000; u2 %=     1000;
      case  6: *(*p)++ = '0' + u2 /      100; u2 %=      100;
      case  5: *(*p)++ = '0' + u2 /       10; u2 %=       10;
      case  4: *(*p)++ = '0' + u2;
      case  3: *(*p)++ = '0' + u3 /     1000; u3 %=     1000;
      case  2: *(*p)++ = '0' + u3 /      100; u3 %=      100;
      case  1: *(*p)++ = '0' + u3 /       10; u3 %=       10;
      case  0: *(*p)++ = '0' + u3;
    }
    *(*p)++ = c;
  }

  static inline void fio_w_Itod(long long int i, char** const p, char const c) {
    if (i < 0) {
      *(*p)++ = '-';
      i = -i;
    }
    fio_w_Utod(i, p, c);
  }

  static inline void fio_w_Utox(unsigned long long int uu, char** const p, char const c) {
    unsigned int u;
    unsigned int d;
    u = (unsigned int) (uu >> 32);
    *(*p)++ = '0' + ((d =   u >> 28        ) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 24) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 20) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 16) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 12) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >>  8) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >>  4) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d =   u        & 0xF ) <= 9 ? d : d + 7);
    u = (unsigned int) uu;
    *(*p)++ = '0' + ((d =   u >> 28        ) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 24) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 20) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 16) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >> 12) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >>  8) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d = ((u >>  4) & 0xF)) <= 9 ? d : d + 7);
    *(*p)++ = '0' + ((d =   u        & 0xF ) <= 9 ? d : d + 7);
    *(*p)++ = c;
  }

  static inline void fio_w_Utox_0x(unsigned long long int u, char** const p, char const c) {
    *(*p)++ = '0';
    *(*p)++ = 'x';
    fio_w_Utox(u, p, c);
  }

  static inline void fio_w_Utox_pg(unsigned long long int u, char** const p, char const c) {
    *(*p)++ = '\\';
    *(*p)++ = 'x';
    fio_w_Utox(u, p, c);
  }

  static inline void fio_w_ctos(char const* a, char** const p, char const c) {
    strcpy(*p, a);
    *p += strlen(a);
    *(*p)++ = c;
  }

  c_only(static) inline void fio_w_init(c_only(fio_writer* this)) {
    char* wb;
#ifdef _WIN32
    HANDLE fh;
#else
    int    fd;
#endif

    if (this->wb_f == NULL) {
      errno = EINVAL;
      perror("w_init error");
      exit(1);
    }

    // Ringpuffer aufbauen
    this->wb_size = this->wb_buffer_size * this->wb_buffer_count;
    if (MemoryMapTwice(wb, this->wb_size, this->wb_begin =,)) {
      Perror("memory error");
      exit(1);
    }
    // Ringpuffer einrichten
    this->wb_end = this->wb_begin + this->wb_size;
    this->wb_start = this->wb_begin;
    this->wb_stop = this->wb_begin + this->wb_size / 2;
    this->wb_pos = this->wb_begin;
    setvbuf(this->wb_f, NULL, _IONBF, 0);
  }

  c_only(static) inline void fio_w(c_only(fio_writer* this)) {
    if (this->wb_pos >= this->wb_stop) {
      // wenn der Pufferbereich (über)voll ist: gesamten Pufferbereich auf Platte schreiben
      size_t const buf_len = (this->wb_pos - this->wb_start) & ~(this->wb_buffer_size - 1);
      if (fwrite(this->wb_start, 1, buf_len, this->wb_f) != buf_len) {
        perror("write error");
        exit(1);
      }
      // Pufferbereich umschalten
      if (this->wb_pos >= this->wb_end) {
        // erster Pufferbereich
        this->wb_start = this->wb_begin;
        this->wb_pos -= this->wb_size;
      }
      else {
        // nächster Pufferbereich
        this->wb_start += buf_len;
      }
      this->wb_stop = this->wb_start + this->wb_buffer_size;
    }
  }

  c_only(static) inline void fio_flush(c_only(fio_writer* this)) {
    if (this->wb_pos > this->wb_start) {
      // wenn noch Daten in Ausgabepuffer sind: Daten auf Platte schreiben
      if (   fwrite(this->wb_start, 1, this->wb_pos - this->wb_start, this->wb_f)
          != this->wb_pos - this->wb_start) {
        perror("write error");
        exit(1);
      }
    }
  }

  c_only(static) inline void fio_w_exit(c_only(fio_writer* this)) {
    fio_flush(c_only(this));
    MemoryUnmapBoth(this->wb_begin, this->wb_size);
  }

#ifdef __cplusplus

  static inline void fio_write_base(unsigned int const u, char*& p, char const c) {
    fio_w_utod(u, &p, c);
  }

  static inline void fio_write_base(int const i, char*& p, char const c) {
    fio_w_itod(i, &p, c);
  }

  static inline void fio_write_base(fio::x const h, char*& p, char const c) {
    fio_w_utox((unsigned int) h, &p, c);
  }

  static inline void fio_write_base(fio::x0x const h, char*& p, char const c) {
    fio_w_utox_0x((unsigned int) h, &p, c);
  }

  static inline void fio_write_base(fio::xpg const h, char*& p, char const c) {
    fio_w_utox_pg((unsigned int) h, &p, c);
  }

  static inline void fio_write_base(unsigned long long int const u, char*& p, char const c) {
    fio_w_Utod(u, &p, c);
  }

  static inline void fio_write_base(long long int const i, char*& p, char const c) {
    fio_w_Itod(i, &p, c);
  }

  static inline void fio_write_base(fio::llx const h, char*& p, char const c) {
    fio_w_Utox((unsigned long long int) h, &p, c);
  }

  static inline void fio_write_base(fio::llx0x const h, char*& p, char const c) {
    fio_w_Utox_0x((unsigned long long int) h, &p, c);
  }

  static inline void fio_write_base(fio::llxpg const h, char*& p, char const c) {
    fio_w_Utox_pg((unsigned long long int) h, &p, c);
  }

  static inline void fio_write_base(char const* a, char*& p, char const c) {
    fio_w_ctos(a, &p, c);
  }

public:
  fio_writer(FILE* f = stdout, size_t buffer_size_factor = 4, size_t buffer_count = 2)
    : wb_f(f), wb_buffer_size (65536 << buffer_size_factor),
      wb_buffer_count(buffer_count), wb_size((65536 << buffer_size_factor) * buffer_count) {
    fio_w_init();
  }

  ~fio_writer() {
    fio_w_exit();
  }

  template <typename T>
  void fio_write(T const v) {
    fio_write_base(v, wb_pos, '\n');
    fio_w();
  }

  template <typename T1, typename T2, typename ... Args>
  void fio_write(T1 const v1, T2 const v2, Args const ...args) {
    fio_write_base(v1, wb_pos, ';');
    fio_write(v2, args...);
  }
};

#else

#define fio_writer(...) __VFUNC__(fio_writer, __VA_ARGS__)

#define fio_writer1(t)       fio_writer t = {stdout, 65536 << 4, 2}
#define fio_writer2(t,f)     fio_writer t = {f, 65536 << 4, 2}
#define fio_writer3(t,f,s)   fio_writer t = {f, 65536 << s, 2}
#define fio_writer4(t,f,s,c) fio_writer t = {f, 65536 << s, c}

#define fio_init_w(t) fio_w_init(&t)
#define fio_exit_w(t) fio_w_exit(&t)

#define fio_write(...) __VFUNC__(fio_write, __VA_ARGS__)

#define fio_write1(t) \
  (*t.wb_pos++ = '\n', \
   fio_w(&t))
#define fio_write3(t,f1,v1) \
  (fio_w_##f1(v1,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write5(t,f1,v1,f2,v2) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write7(t,f1,v1,f2,v2,f3,v3) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write9(t,f1,v1,f2,v2,f3,v3,f4,v4) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write11(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,';'), \
   fio_w_##f5(v5,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write13(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,';'), \
   fio_w_##f5(v5,&t.wb_pos,';'), \
   fio_w_##f6(v6,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write15(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,';'), \
   fio_w_##f5(v5,&t.wb_pos,';'), \
   fio_w_##f6(v6,&t.wb_pos,';'), \
   fio_w_##f7(v7,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write17(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7,f8,v8) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,';'), \
   fio_w_##f5(v5,&t.wb_pos,';'), \
   fio_w_##f6(v6,&t.wb_pos,';'), \
   fio_w_##f7(v7,&t.wb_pos,';'), \
   fio_w_##f8(v8,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write19(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7,f8,v8,f9,v9) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,';'), \
   fio_w_##f5(v5,&t.wb_pos,';'), \
   fio_w_##f6(v6,&t.wb_pos,';'), \
   fio_w_##f7(v7,&t.wb_pos,';'), \
   fio_w_##f8(v8,&t.wb_pos,';'), \
   fio_w_##f9(v9,&t.wb_pos,'\n'), \
   fio_w(&t))
#define fio_write21(t,f1,v1,f2,v2,f3,v3,f4,v4,f5,v5,f6,v6,f7,v7,f8,v8,f9,v9,fa,va) \
  (fio_w_##f1(v1,&t.wb_pos,';'), \
   fio_w_##f2(v2,&t.wb_pos,';'), \
   fio_w_##f3(v3,&t.wb_pos,';'), \
   fio_w_##f4(v4,&t.wb_pos,';'), \
   fio_w_##f5(v5,&t.wb_pos,';'), \
   fio_w_##f6(v6,&t.wb_pos,';'), \
   fio_w_##f7(v7,&t.wb_pos,';'), \
   fio_w_##f8(v8,&t.wb_pos,';'), \
   fio_w_##f9(v9,&t.wb_pos,';'), \
   fio_w_##fa(va,&t.wb_pos,'\n'), \
   fio_w(&t))

#define ud(x) utod, x
#define id(x) itod, x
#define ux(x) utox, x
#define ux_0x(x) utox_0x, x
#define ux_pg(x) utox_pg, x
#define Ud(x) Utod, x
#define Id(x) Itod, x
#define Ux(x) Utox, x
#define Ux_0x(x) Utox_0x, x
#define Ux_pg(x) Utox_pg, x
#define cs(x) ctos, x

#endif

#endif
