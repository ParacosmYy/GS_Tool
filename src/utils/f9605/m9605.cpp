#include "f9605/m9605.h"
QVector<double> m9605::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
