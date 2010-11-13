#!/bin/sh

gcc -c ../libmpeg2/cpu_accel.c -o cpu_accel.obj -I../include  -I../vc++ -Wall -Werror 
gcc -c ../libmpeg2/cpu_state.c -o cpu_state.obj -I../include  -I../vc++ -Wall -Werror 
gcc -c ../libmpeg2/idct_mmx.c -o idct_mmx.obj -I../include  -I../vc++ -Wall -Werror 
gcc -c ../libmpeg2/motion_comp_mmx.c -o motion_comp_mmx.obj -I../include  -I../vc++ -Wall -Werror
