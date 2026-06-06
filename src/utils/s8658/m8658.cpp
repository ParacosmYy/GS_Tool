#include "s8658/m8658.h"
QVector<double> m8658::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
