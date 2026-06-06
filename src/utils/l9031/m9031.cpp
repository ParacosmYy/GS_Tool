#include "l9031/m9031.h"
QVector<double> m9031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
