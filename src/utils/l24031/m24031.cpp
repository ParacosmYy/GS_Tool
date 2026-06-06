#include "l24031/m24031.h"
QVector<double> m24031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
