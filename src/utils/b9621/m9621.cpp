#include "b9621/m9621.h"
QVector<double> m9621::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
