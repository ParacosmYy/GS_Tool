#include "l29811/m29811.h"
QVector<double> m29811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
