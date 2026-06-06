#include "p35595/m35595.h"
QVector<double> m35595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
