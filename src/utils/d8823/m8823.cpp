#include "d8823/m8823.h"
QVector<double> m8823::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
