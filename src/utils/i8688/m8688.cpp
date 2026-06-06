#include "i8688/m8688.h"
QVector<double> m8688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
