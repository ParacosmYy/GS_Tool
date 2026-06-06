#include "k15130/m15130.h"
QVector<double> m15130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
