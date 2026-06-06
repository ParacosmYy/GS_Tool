#include "h7827/m7827.h"
QVector<double> m7827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
