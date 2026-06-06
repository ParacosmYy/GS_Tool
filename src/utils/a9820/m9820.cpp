#include "a9820/m9820.h"
QVector<double> m9820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
