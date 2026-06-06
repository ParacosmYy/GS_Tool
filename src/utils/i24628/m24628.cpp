#include "i24628/m24628.h"
QVector<double> m24628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
