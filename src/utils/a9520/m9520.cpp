#include "a9520/m9520.h"
QVector<double> m9520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
