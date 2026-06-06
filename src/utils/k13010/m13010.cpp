#include "k13010/m13010.h"
QVector<double> m13010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
