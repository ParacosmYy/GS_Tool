#include "l19911/m19911.h"
QVector<double> m19911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
