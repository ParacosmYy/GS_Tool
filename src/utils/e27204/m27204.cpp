#include "e27204/m27204.h"
QVector<double> m27204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
