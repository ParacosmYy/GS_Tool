#include "m12232/m12232.h"
QVector<double> m12232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
