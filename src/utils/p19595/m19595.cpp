#include "p19595/m19595.h"
QVector<double> m19595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
