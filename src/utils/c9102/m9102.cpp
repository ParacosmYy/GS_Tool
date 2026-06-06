#include "c9102/m9102.h"
QVector<double> m9102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
