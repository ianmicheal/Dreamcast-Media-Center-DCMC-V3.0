#ifndef MATH_VECTOR
#define MATH_VECTOR

inline void CopyVector3f( vector3f in, vector3f out )
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

inline void CopyVector2f( vector2f in, vector2f out )
{
    out[0] = in[0];
    out[1] = in[1];
}

#endif
