#include "i16708/m16708.h"
QVector<double> m16708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
