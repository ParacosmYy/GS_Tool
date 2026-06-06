#include "l37511/m37511.h"
QVector<double> m37511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
