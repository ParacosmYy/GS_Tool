#include "o25914/m25914.h"
QVector<double> m25914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
