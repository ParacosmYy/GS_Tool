#include "m25272/m25272.h"
QVector<double> m25272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
