#include "a27260/m27260.h"
QVector<double> m27260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
