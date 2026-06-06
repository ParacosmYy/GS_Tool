#include "p24255/m24255.h"
QVector<double> m24255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
