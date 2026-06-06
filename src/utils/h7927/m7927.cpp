#include "h7927/m7927.h"
QVector<double> m7927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
