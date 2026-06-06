#include "a9300/m9300.h"
QVector<double> m9300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
