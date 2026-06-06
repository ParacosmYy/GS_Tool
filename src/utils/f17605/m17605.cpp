#include "f17605/m17605.h"
QVector<double> m17605::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
