#include "m24672/m24672.h"
QVector<double> m24672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
