#include "a25540/m25540.h"
QVector<double> m25540::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
