#include "b28601/m28601.h"
QVector<double> m28601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
