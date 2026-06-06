#include "i8768/m8768.h"
QVector<double> m8768::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
