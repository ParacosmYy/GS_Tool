#include "k25650/m25650.h"
QVector<double> m25650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
