#include "p9595/m9595.h"
QVector<double> m9595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
