#include "a18300/m18300.h"
QVector<double> m18300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
