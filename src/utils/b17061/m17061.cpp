#include "b17061/m17061.h"
QVector<double> m17061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
