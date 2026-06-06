#include "a17380/m17380.h"
QVector<double> m17380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
