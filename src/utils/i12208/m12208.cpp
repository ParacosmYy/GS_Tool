#include "i12208/m12208.h"
QVector<double> m12208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
