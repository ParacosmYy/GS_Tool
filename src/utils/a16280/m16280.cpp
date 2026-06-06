#include "a16280/m16280.h"
QVector<double> m16280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
