#ifndef PAT_H
#define PAT_H

#define IA32_PAT_MSR    631

#define F_PWT           (1ULL << 3)
#define F_PCD           (1ULL << 4)
#define F_PAT_4K        (1ULL << 7)

#define F_CACHE_WB      0
#define F_CACHE_WC      F_PWT
#define F_CACHE_UC      (F_PCD)

#define PAT_WB          0
#define PAT_WC          1

void pat_init(void);

#endif // PAT_H