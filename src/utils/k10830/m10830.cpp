#include "k10830/m10830.h"
QVector<double> m10830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
