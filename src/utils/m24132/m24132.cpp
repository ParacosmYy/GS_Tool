#include "m24132/m24132.h"
QVector<double> m24132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
