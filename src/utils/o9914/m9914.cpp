#include "o9914/m9914.h"
QVector<double> m9914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
