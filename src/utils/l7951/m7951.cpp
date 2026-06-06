#include "l7951/m7951.h"
QVector<double> m7951::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
