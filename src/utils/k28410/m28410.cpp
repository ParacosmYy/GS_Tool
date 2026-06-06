#include "k28410/m28410.h"
QVector<double> m28410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
