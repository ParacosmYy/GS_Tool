#include "l8271/m8271.h"
QVector<double> m8271::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
