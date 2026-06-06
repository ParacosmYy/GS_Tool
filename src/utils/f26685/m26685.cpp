#include "f26685/m26685.h"
QVector<double> m26685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
