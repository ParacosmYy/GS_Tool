#include "e28064/m28064.h"
QVector<double> m28064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
