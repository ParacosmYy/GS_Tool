#include "i16168/m16168.h"
QVector<double> m16168::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
