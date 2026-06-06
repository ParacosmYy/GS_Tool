#include "k24330/m24330.h"
QVector<double> m24330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
