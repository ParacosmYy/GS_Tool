#include "a24520/m24520.h"
QVector<double> m24520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
