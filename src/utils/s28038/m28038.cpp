#include "s28038/m28038.h"
QVector<double> m28038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
