#include "b16801/m16801.h"
QVector<double> m16801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
