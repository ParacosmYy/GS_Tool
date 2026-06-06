#include "a27500/m27500.h"
QVector<double> m27500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
