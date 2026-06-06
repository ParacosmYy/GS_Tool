#include "h16567/m16567.h"
QVector<double> m16567::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
