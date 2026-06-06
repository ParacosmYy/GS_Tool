#include "b17101/m17101.h"
QVector<double> m17101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
