#include "g18126/m18126.h"
QVector<double> m18126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
