#include "keyboard.h"
#include "../memory/status.h"
#include "../task/process.h"
#include "../task/task.h"
#include "PS2.h"

static struct keyboard *keyboard_list_head = 0;
static struct keyboard *keyboard_list_last = 0;

void keyboard_init(){
  keyboard_insert(PS2_init());
}


int keyboard_insert(struct keyboard *keyboard){
  if (!keyboard || keyboard->init == 0)
    return -EINVARG;
  if (keyboard_list_last){
    keyboard_list_last->next = keyboard;
    keyboard_list_last = keyboard;
  } else {
    keyboard_list_head = keyboard;
    keyboard_list_last = keyboard;
  }

  return keyboard->init();
}

static inline int keyboard_get_tail_index(struct process *process){
  return process->keyboard.tail % sizeof(process->keyboard.buffer);
}

void keyboard_backspace(struct process *process){
  process->keyboard.tail--;
  int real_index = keyboard_get_tail_index(process);
  process->keyboard.buffer[real_index] = 0;
}

void keyboard_push(char c){
  struct process *process = task_current()->process;
  if (!process)
    return;
  int real_index = keyboard_get_tail_index(process);
  process->keyboard.buffer[real_index] = c;
  process->keyboard.tail++;
}

char keyboard_pop(){
  if (!task_current())
    return 0;
  struct process *process = task_current()->process;
  int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
  char c = process->keyboard.buffer[real_index];
  if (c == 0)
    return 0;

  process->keyboard.buffer[real_index] = 0;
  process->keyboard.head++;
  return c;
}
