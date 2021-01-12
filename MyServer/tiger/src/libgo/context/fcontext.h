
#include <stdint.h>
#include <sys/types.h>
namespace tiger {

//借用boost 库
extern "C"{

typedef void* fcontext_t;
typedef void ( *fn_t)(intptr_t);
//上下文的切换
intptr_t jump_fcontext(fcontext_t * ofc, fcontext_t nfc,
        intptr_t vp, bool preserve_fpu = false);
//上下文的创建
fcontext_t make_fcontext(void* stack, size_t size, fn_t fn);

}
}
