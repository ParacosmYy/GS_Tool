#include "r9017/m9017.h"
QVector<double> m9017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
