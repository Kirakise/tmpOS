#include "io.h"
#include "../task/task.h"
#include "../print/print.h"

void *isr80h_command1_print(struct interrupt_frame *frame){
  void *str = task_get_stack_item(task_current(), 0);
  char buf[1024];

  copy_string_from_task(task_current(), str, buf, sizeof(buf));
  printk("%s", buf);
  return 0;
}
