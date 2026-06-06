#include "c16842/m16842.h"
QVector<double> m16842::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
