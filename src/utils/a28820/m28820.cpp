#include "a28820/m28820.h"
QVector<double> m28820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
