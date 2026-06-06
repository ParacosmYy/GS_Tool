#include "b15101/m15101.h"
QVector<double> m15101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
