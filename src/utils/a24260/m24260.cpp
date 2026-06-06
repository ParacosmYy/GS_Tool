#include "a24260/m24260.h"
QVector<double> m24260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
