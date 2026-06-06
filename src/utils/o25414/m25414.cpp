#include "o25414/m25414.h"
QVector<double> m25414::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
