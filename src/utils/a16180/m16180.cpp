#include "a16180/m16180.h"
QVector<double> m16180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
