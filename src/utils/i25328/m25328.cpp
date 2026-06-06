#include "i25328/m25328.h"
QVector<double> m25328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
