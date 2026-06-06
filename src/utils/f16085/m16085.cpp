#include "f16085/m16085.h"
QVector<double> m16085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
