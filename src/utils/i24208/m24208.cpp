#include "i24208/m24208.h"
QVector<double> m24208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
