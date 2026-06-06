#include "k9790/m9790.h"
QVector<double> m9790::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
