#include "m30552/m30552.h"
QVector<double> m30552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
