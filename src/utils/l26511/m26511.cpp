#include "l26511/m26511.h"
QVector<double> m26511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
