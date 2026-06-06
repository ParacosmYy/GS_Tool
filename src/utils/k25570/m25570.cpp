#include "k25570/m25570.h"
QVector<double> m25570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
