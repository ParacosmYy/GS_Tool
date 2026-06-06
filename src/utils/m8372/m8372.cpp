#include "m8372/m8372.h"
QVector<double> m8372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
