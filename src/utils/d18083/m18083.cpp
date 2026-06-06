#include "d18083/m18083.h"
QVector<double> m18083::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
