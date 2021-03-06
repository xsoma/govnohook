#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
// Parameters:
//
//   float amount;
//   sampler2D texSampler;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   amount       c0       1
//   texSampler   s0       1
//

    ps_2_0
    def c1, 1, 0, 0, 0
    dcl t0.xy  // uv<0,1>
    dcl_2d s0

#line 7 "F:\1necheats\GOESP-master\GOESP\Resources\Shaders\chromatic_aberration.hlsl"
    mov r0.xy, -c0.x
    add r0.xy, r0, t0
    add r1.xy, t0, c0.x
    texld r0, r0, s0
    texld r2, t0, s0
    texld r1, r1, s0
    mov r1.x, r0.x  // ::r<0>
    mov r1.y, r2.y  // ::g<0>
    mov r1.z, r1.z  // ::b<0>
    mov r1.x, r1.x  // ::main<0>
    mov r1.y, r1.y  // ::main<1>
    mov r1.z, r1.z  // ::main<2>
    mov r1.w, c1.x  // ::main<3>

#line 5
    mov oC0, r1  // ::main<0,1,2,3>

// approximately 14 instruction slots used (3 texture, 11 arithmetic)
#endif

const BYTE chromatic_aberration[] =
{
      0,   2, 255, 255, 254, 255, 
    145,   0,  68,  66,  85,  71, 
     40,   0,   0,   0,  24,   2, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 116,   0, 
      0,   0,  17,   0,   0,   0, 
    120,   0,   0,   0,   5,   0, 
      0,   0, 180,   1,   0,   0, 
     48,   1,   0,   0,  70,  58, 
     92,  49, 110, 101,  99, 104, 
    101,  97, 116, 115,  92,  71, 
     79,  69,  83,  80,  45, 109, 
     97, 115, 116, 101, 114,  92, 
     71,  79,  69,  83,  80,  92, 
     82, 101, 115, 111, 117, 114, 
     99, 101, 115,  92,  83, 104, 
     97, 100, 101, 114, 115,  92, 
     99, 104, 114, 111, 109,  97, 
    116, 105,  99,  95,  97,  98, 
    101, 114, 114,  97, 116, 105, 
    111, 110,  46, 104, 108, 115, 
    108,   0,  40,   0,   0,   0, 
      0,   0, 255, 255, 252,   2, 
      0,   0,   0,   0, 255, 255, 
     20,   3,   0,   0,   0,   0, 
    255, 255,  32,   3,   0,   0, 
      7,   0,   0,   0,  44,   3, 
      0,   0,   7,   0,   0,   0, 
     56,   3,   0,   0,   9,   0, 
      0,   0,  72,   3,   0,   0, 
      7,   0,   0,   0,  88,   3, 
      0,   0,   8,   0,   0,   0, 
    104,   3,   0,   0,   9,   0, 
      0,   0, 120,   3,   0,   0, 
      7,   0,   0,   0, 136,   3, 
      0,   0,   8,   0,   0,   0, 
    148,   3,   0,   0,   9,   0, 
      0,   0, 160,   3,   0,   0, 
     11,   0,   0,   0, 172,   3, 
      0,   0,  11,   0,   0,   0, 
    184,   3,   0,   0,  11,   0, 
      0,   0, 196,   3,   0,   0, 
     11,   0,   0,   0, 208,   3, 
      0,   0,   5,   0,   0,   0, 
    220,   3,   0,   0,  98,   0, 
    171, 171,   0,   0,   3,   0, 
      1,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     11,   0,   0,   0, 255, 255, 
    255, 255,   0,   0, 255, 255, 
    103,   0, 171, 171,  10,   0, 
      0,   0, 255, 255,   0,   0, 
    255, 255, 255, 255, 109,  97, 
    105, 110,   0, 171, 171, 171, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,  12,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255, 255, 255,  13,   0, 
      0,   0, 255, 255,   1,   0, 
    255, 255, 255, 255,  14,   0, 
      0,   0, 255, 255, 255, 255, 
      2,   0, 255, 255,  15,   0, 
      0,   0, 255, 255, 255, 255, 
    255, 255,   3,   0,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      2,   0,   3,   0, 114,   0, 
    171, 171,   9,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
    255, 255, 117, 118,   0, 171, 
      1,   0,   3,   0,   1,   0, 
      2,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   1,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,   0,   1,   0,   0, 
      4,   1,   0,   0,   1,   0, 
      0,   0,  20,   1,   0,   0, 
      0,   0,   0,   0,  32,   1, 
      0,   0,   4,   1,   0,   0, 
      1,   0,   0,   0,  36,   1, 
      0,   0,   0,   0,   0,   0, 
     48,   1,   0,   0,  56,   1, 
      0,   0,   5,   0,   0,   0, 
     72,   1,   0,   0,   0,   0, 
      0,   0, 132,   1,   0,   0, 
      4,   1,   0,   0,   1,   0, 
      0,   0, 136,   1,   0,   0, 
     48,   1,   0,   0, 148,   1, 
      0,   0, 152,   1,   0,   0, 
      1,   0,   0,   0, 168,   1, 
      0,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  49,  48,  46,  49,   0, 
    254, 255,  43,   0,  67,  84, 
     65,  66,  28,   0,   0,   0, 
    127,   0,   0,   0,   0,   2, 
    255, 255,   2,   0,   0,   0, 
     28,   0,   0,   0,   5,   1, 
      0,   0, 120,   0,   0,   0, 
     68,   0,   0,   0,   2,   0, 
      0,   0,   1,   0,   0,   0, 
     76,   0,   0,   0,   0,   0, 
      0,   0,  92,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0, 104,   0,   0,   0, 
      0,   0,   0,   0,  97, 109, 
    111, 117, 110, 116,   0, 171, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 116, 101, 
    120,  83,  97, 109, 112, 108, 
    101, 114,   0, 171,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 112, 115,  95,  50, 
     95,  48,   0,  77, 105,  99, 
    114, 111, 115, 111, 102, 116, 
     32,  40,  82,  41,  32,  72, 
     76,  83,  76,  32,  83, 104, 
     97, 100, 101, 114,  32,  67, 
    111, 109, 112, 105, 108, 101, 
    114,  32,  49,  48,  46,  49, 
      0, 171,  81,   0,   0,   5, 
      1,   0,  15, 160,   0,   0, 
    128,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  31,   0,   0,   2, 
      0,   0,   0, 128,   0,   0, 
      3, 176,  31,   0,   0,   2, 
      0,   0,   0, 144,   0,   8, 
     15, 160,   1,   0,   0,   2, 
      0,   0,   3, 128,   0,   0, 
      0, 161,   2,   0,   0,   3, 
      0,   0,   3, 128,   0,   0, 
    228, 128,   0,   0, 228, 176, 
      2,   0,   0,   3,   1,   0, 
      3, 128,   0,   0, 228, 176, 
      0,   0,   0, 160,  66,   0, 
      0,   3,   0,   0,  15, 128, 
      0,   0, 228, 128,   0,   8, 
    228, 160,  66,   0,   0,   3, 
      2,   0,  15, 128,   0,   0, 
    228, 176,   0,   8, 228, 160, 
     66,   0,   0,   3,   1,   0, 
     15, 128,   1,   0, 228, 128, 
      0,   8, 228, 160,   1,   0, 
      0,   2,   1,   0,   1, 128, 
      0,   0,   0, 128,   1,   0, 
      0,   2,   1,   0,   2, 128, 
      2,   0,  85, 128,   1,   0, 
      0,   2,   1,   0,   4, 128, 
      1,   0, 170, 128,   1,   0, 
      0,   2,   1,   0,   1, 128, 
      1,   0,   0, 128,   1,   0, 
      0,   2,   1,   0,   2, 128, 
      1,   0,  85, 128,   1,   0, 
      0,   2,   1,   0,   4, 128, 
      1,   0, 170, 128,   1,   0, 
      0,   2,   1,   0,   8, 128, 
      1,   0,   0, 160,   1,   0, 
      0,   2,   0,   8,  15, 128, 
      1,   0, 228, 128, 255, 255, 
      0,   0
};
