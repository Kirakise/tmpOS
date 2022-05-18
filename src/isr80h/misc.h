#pragma once
#include "../idt/idt.h"
#include "isr80h.h"

void *isr80h_command0_sum(struct interrupt_frame *frame);
