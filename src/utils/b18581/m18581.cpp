#include "b18581/m18581.h"
QVector<double> m18581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
