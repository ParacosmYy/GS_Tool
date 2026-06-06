#include "k25770/m25770.h"
QVector<double> m25770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
