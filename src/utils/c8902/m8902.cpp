#include "c8902/m8902.h"
QVector<double> m8902::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
