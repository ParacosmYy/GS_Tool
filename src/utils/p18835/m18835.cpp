#include "p18835/m18835.h"
QVector<double> m18835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
