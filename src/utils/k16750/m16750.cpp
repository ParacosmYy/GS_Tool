#include "k16750/m16750.h"
QVector<double> m16750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
