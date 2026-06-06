#include "a24300/m24300.h"
QVector<double> m24300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
