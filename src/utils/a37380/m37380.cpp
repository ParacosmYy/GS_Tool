#include "a37380/m37380.h"
QVector<double> m37380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
