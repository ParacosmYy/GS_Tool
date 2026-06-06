#include "d7823/m7823.h"
QVector<double> m7823::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
