#include "i16768/m16768.h"
QVector<double> m16768::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
