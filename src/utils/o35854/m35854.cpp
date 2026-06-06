#include "o35854/m35854.h"
QVector<double> m35854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
