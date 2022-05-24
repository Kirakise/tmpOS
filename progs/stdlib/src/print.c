#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "os.h"

static uint32_t strlen_3(const char *s){
  uint32_t i = 0;
  while (*s++)
    i++;
  return i;
}

static char *strdup_3(const char *str){
  uint32_t size = strlen_3(str);
  char *ret = malloc(size);
  for (int i = 0; i < size; i++){
    ret[i] = str[i];
  }
  ret[size] = 0;
  return ret;
}

char* itoa(int base, int num){
        int copy = num;
        int size = 0;
        if (base > 32)
                return 0;
        if (copy == 0)
                return strdup_3("0");
        while (copy){
                size++;
                copy /= base;
        }
        char *ret = malloc(size + 1);
        if (!ret)
                return 0;
        ret[size] = 0;
        copy = num > 0 ? num : -num;
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


char* itoau(int base, uint32_t num){
        uint32_t copy = num;
        int size = 0;
        if (base > 32)
                return 0;
        if (copy == 0)
                return strdup_3("0");
        while (copy){
                size++;
                copy /= base;
        }
        char *ret = malloc(size + 1);
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

static size_t strlen(const char *s){
        uint32_t i = 0;
        while (s[i])
          i++;
        return i;
}

static char *makeaddr(const char *s){
        uint32_t len = strlen(s);
        char *ret = malloc(11);
        ret[10] = 0;
        ret[0] = '0';
        ret[1] = 'x';
        int i, j = 0;
        for (i = 0; i < 8 - len; i++)
                ret[i + 2] = '0';
        while (len-- > 0)
                ret[i++ + 2] = s[j++];
        return ret;
}


static char *getarg(const char *s, va_list *lst){
        if (s[1] == 'i' || s[1] == 'd')
                return itoa(10, va_arg(*lst, int));
        else if (s[1] == '%')
                return strdup_3("%");
        else if (s[1] == 'X')
                return itoa(16, va_arg(*lst, int));
        else if (s[1] == 's')
                return strdup_3(va_arg(*lst, char *));
        else if (s[1] == 'c'){
                char c[2];
                c[1] = 0;
                c[0] = (char)va_arg(*lst, uint32_t);
                return strdup_3(c);
        }
        else if (s[1] == 'p'){
                char *tmp = itoau(16, va_arg(*lst, uint32_t));
                char *tmp2 = makeaddr(tmp);
                free(tmp);
                return tmp2;
        }
        return 0;
}

static int printarg(char *s){
        if (s == 0)
                return 0;
        int i = 0;
        const char *tmp = s;
        while (*tmp++)
          i++;
        print(s);
        free(s);
        return i;
}

int printf(const char *form, ...){
        va_list lst;
        if (!form)
                return -1;
        va_start(lst, form);
        int i = 0;
        while (*form){
                if (*form != '%'){
                        os_putchar(*form++);
                        i++;
                }
                else{
                        i += printarg(getarg(form, &lst));
                        form += 2;
                }
        }
        return i;
}
