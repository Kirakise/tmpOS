#include "print.h"
#include "../utils.h"
#include "../memory/kheap.h"
#include <stdarg.h>
#include "../memory/status.h"
#include "../kernel.h"
#include "../task/tss.h"
static char* itoa(int base, int num){
        int copy = num;
        int size = 0;
        if (base > 32)
                return 0;
        if (copy == 0)
                return strdup("0");
        while (copy){
                size++;
                copy /= base;
        }
        char *ret = kmalloc(size + 1);
        if (!ret)
                return 0;
        ret[size] = 0;
        copy = num > 0 ? num : -num;
        while (--size){
                int tmp = copy % base;
                if (tmp < 10)
                        ret[size] = tmp + '0'; 
                else
                        ret[size] = tmp - 10 + 'A';
                copy /= base;
        }
        return ret;
}


static char* itoau(int base, uint32_t num){
        uint32_t copy = num;
        int size = 0;
        if (base > 32)
                return 0;
        if (copy == 0)
                return strdup("0");
        while (copy){
                size++;
                copy /= base;
        }
        char *ret = kmalloc(size + 1);
        if (!ret)
                return 0;
        ret[size] = 0;
        copy = num;
        while (--size >= 0){
                int tmp = copy % base;
                if (tmp < 10)
                        ret[size] = tmp + '0'; 
                else
                        ret[size] = tmp - 10 + 'A';
                copy /= base;
        }
        return ret;
}

static char *makeaddr(const char *s){
        uint32_t len = strlen(s);
        char *ret = kmalloc(11);
        ret[10] = 0;
        ret[0] = '0';
        ret[1] = 'x';
        int i, j = 0;
        for (i = 0; i < 8 - len; i++)
                ret[i + 2] = '0';
        while (--len > 0)
                ret[i++ + 2] = s[j++];
        return ret;
}


static char *getarg(const char *s, va_list *lst){
        if (s[1] == 'i' || s[1] == 'd')
                return itoa(10, va_arg(*lst, int));
        else if (s[1] == '%')
                return strdup("%");
        else if (s[1] == 'X')
                return itoa(16, va_arg(*lst, int));
        else if (s[1] == 's')
                return strdup(va_arg(*lst, char *));
        else if (s[1] == 'c'){
                char c = (char)va_arg(*lst, uint8_t);
                return strdup(&c);
        }
        else if (s[1] == 'p'){
                char *tmp = itoau(16, va_arg(*lst, uint32_t));
                char *tmp2 = makeaddr(tmp);
                kfree(tmp);
                return tmp2;
        }
        return 0;
}

static int printarg(char *s){
        if (s == 0)
                return 0;
        int i = 0;
        const char *tmp = s;
        while (*tmp && ++i)
                terminal_writechar(*tmp++, RED, BLACK);
        kfree(s);
        return i;
}

int printk(const char *form, ...){
        va_list lst;
        if (!form)
                return -EINVARG;
        va_start(lst, form);
        int i = 0;
        while (*form){
                if (*form != '%')
                        i += kwrite(1, (char *)form++, 1);
                else{
                        i += printarg(getarg(form, &lst));
                        form += 2;
                }
        }
        return i;
}

void print_stack(int max_items){
        int res = 0;
        uint32_t arr[max_items];
        res = walk_stack(arr, max_items);
        for (int i = 0; i < res; i++)
                printk("%p\n", arr[i]); 
}
