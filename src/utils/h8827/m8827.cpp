#include "h8827/m8827.h"
QVector<double> m8827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
