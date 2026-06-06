#include "m26012/m26012.h"
QVector<double> m26012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
