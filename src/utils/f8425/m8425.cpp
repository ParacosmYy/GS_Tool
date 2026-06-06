#include "f8425/m8425.h"
QVector<double> m8425::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
