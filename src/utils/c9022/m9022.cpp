#include "c9022/m9022.h"
QVector<double> m9022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
