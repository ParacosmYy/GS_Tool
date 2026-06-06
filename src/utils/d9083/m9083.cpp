#include "d9083/m9083.h"
QVector<double> m9083::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
