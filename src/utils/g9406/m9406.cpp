#include "g9406/m9406.h"
QVector<double> m9406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
