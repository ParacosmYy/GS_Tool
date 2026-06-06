#include "b25061/m25061.h"
QVector<double> m25061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
