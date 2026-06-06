#include "s28118/m28118.h"
QVector<double> m28118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
