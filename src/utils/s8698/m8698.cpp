#include "s8698/m8698.h"
QVector<double> m8698::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
