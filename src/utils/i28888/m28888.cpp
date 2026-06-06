#include "i28888/m28888.h"
QVector<double> m28888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
