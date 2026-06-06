#include "i10868/m10868.h"
QVector<double> m10868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
