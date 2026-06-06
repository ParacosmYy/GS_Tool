#include "c16482/m16482.h"
QVector<double> m16482::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
