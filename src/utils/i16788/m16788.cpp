#include "i16788/m16788.h"
QVector<double> m16788::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
