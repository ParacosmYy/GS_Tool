#include "b25581/m25581.h"
QVector<double> m25581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
