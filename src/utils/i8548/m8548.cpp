#include "i8548/m8548.h"
QVector<double> m8548::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
