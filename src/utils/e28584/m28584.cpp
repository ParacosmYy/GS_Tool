#include "e28584/m28584.h"
QVector<double> m28584::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
