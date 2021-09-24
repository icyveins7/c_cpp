#include <smmintrin.h>
#include <iostream>
#include <stdint.h>

int main(){
    printf("Test\n");
    
    int16_t arr[8];
    for (int i = 0; i < 8; i++){
        arr[i] = 16383;
    } // at this point none of the masked bits are set
    printf("All arr data written as %hd\n", arr[0]);
    
    int16_t mask[8];
    
    for (int i = 0; i < 8; i++){
        mask[i] = 0x1 << 14;    
    }
    printf("%hd written to mask\n", mask[0]);
    
    __m128i carr = _mm_set_epi16(arr[0],arr[1],arr[2],arr[3],arr[4],arr[5],arr[6],arr[7]);
    __m128i marr = _mm_set_epi16(mask[0],mask[1],mask[2],mask[3],mask[4],mask[5],mask[6],mask[7]);
    int c = _mm_testz_si128(carr, marr);
    printf("testz: %d\n", c);
    
    arr[0] = 16384;
    printf("arr[0] now %hd\n", arr[0]);
    carr = _mm_set_epi16(arr[0],arr[1],arr[2],arr[3],arr[4],arr[5],arr[6],arr[7]);
    c = _mm_testz_si128(carr, marr);
    printf("testz: %d\n", c);
    
    
    

    return 0;
}
