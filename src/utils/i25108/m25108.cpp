#include "i25108/m25108.h"
QVector<double> m25108::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
