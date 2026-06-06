#include "l18031/m18031.h"
QVector<double> m18031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
