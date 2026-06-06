#include "a27200/m27200.h"
QVector<double> m27200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
