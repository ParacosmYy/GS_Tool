#include "l8931/m8931.h"
QVector<double> m8931::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
