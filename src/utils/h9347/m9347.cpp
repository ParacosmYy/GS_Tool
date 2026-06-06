#include "h9347/m9347.h"
QVector<double> m9347::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
