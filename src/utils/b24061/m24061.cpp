#include "b24061/m24061.h"
QVector<double> m24061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
