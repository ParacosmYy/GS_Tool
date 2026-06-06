#include "i25168/m25168.h"
QVector<double> m25168::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
