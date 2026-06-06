#include "h9607/m9607.h"
QVector<double> m9607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
