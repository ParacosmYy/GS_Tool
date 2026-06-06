#include "k17050/m17050.h"
QVector<double> m17050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
