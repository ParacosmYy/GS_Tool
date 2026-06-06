#include "k17550/m17550.h"
QVector<double> m17550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
