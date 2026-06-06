#include "m24892/m24892.h"
QVector<double> m24892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
