#include "a24180/m24180.h"
QVector<double> m24180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
