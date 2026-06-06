#include "k32930/m32930.h"
QVector<double> m32930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
