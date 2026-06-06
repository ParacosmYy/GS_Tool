#include "f12485/m12485.h"
QVector<double> m12485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
