#include "c16262/m16262.h"
QVector<double> m16262::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
