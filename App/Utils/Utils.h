#include <cmath>

inline void HSVtoRGB(float H, float S, float V, float &R, float &G, float &B) {
    if(H > 360 || H < 0 || S > 1 || S < 0 || V > 1 || V < 0){
        return;
    }
    
    float C = V * S;
    float X = C * (1 - fabsf(fmodf(H / 60.0, 2) - 1));
    float m = V - C;
    float r, g, b;

    if(H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    } else if(H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    } else if(H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    } else if(H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    } else if(H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }
    
    R = (r + m);
    G = (g + m);
    B = (b + m);
}

#define SINGLE_POLE(in, out, coeff) out += (coeff) * ((in) - out)

#define CONSTRAIN(value, min, max) \
    if (value < (min)) { \
      value = (min); \
    } else if (value > (max)) { \
      value = (max); \
    }
