#include "f25125/m25125.h"
QVector<double> m25125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
