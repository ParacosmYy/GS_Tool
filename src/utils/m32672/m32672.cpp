#include "m32672/m32672.h"
QVector<double> m32672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
