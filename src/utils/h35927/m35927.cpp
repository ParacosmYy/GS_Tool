#include "h35927/m35927.h"
QVector<double> m35927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
