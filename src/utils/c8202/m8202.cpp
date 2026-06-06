#include "c8202/m8202.h"
QVector<double> m8202::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
