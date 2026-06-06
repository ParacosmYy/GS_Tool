#include "i9208/m9208.h"
QVector<double> m9208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
