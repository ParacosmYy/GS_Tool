#include "i24708/m24708.h"
QVector<double> m24708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
