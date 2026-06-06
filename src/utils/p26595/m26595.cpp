#include "p26595/m26595.h"
QVector<double> m26595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
