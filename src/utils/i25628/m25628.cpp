#include "i25628/m25628.h"
QVector<double> m25628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
