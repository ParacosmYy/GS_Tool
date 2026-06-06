#include "p33255/m33255.h"
QVector<double> m33255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
