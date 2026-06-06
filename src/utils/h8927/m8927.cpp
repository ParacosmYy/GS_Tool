#include "h8927/m8927.h"
QVector<double> m8927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
