#ifndef Functions_HLSL
#define Functions_HLSL

float invlerp(in float min, in float max, in float x)
{
    return (x - min) / (max - min);
}
float lstep(in float min, in float max, in float x)
{
    return clamp(invlerp(min, max, x), 0, 1);
}

#endif
