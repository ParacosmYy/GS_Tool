#include "a15260/m15260.h"
QVector<double> m15260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
