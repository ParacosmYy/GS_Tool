#include "i8708/m8708.h"
QVector<double> m8708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
