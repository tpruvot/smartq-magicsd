#ifndef __CONFIG_H_H_
#define __CONFIG_H_H_

#if defined(SMDK6410) || defined(SMARTQ5) || defined(SMARTQ7)
#define MEM_START 0x50000000
#define MEM_SIZE (128*1024*1024)

#ifdef SMDK6410
#define MACHINE_ID 1626
#elif defined(SMARTQ5)
#define MACHINE_ID 2534
#else /* SMARTQ7 */
#define MACHINE_ID 2479
#endif

#else /* SMDK2440 */
#define MEM_START 0x30000000
#define MEM_SIZE  (64*1024*1024)
#define MACHINE_ID 362

#endif

#endif
