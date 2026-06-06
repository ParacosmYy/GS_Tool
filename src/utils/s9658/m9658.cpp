#include "s9658/m9658.h"
QVector<double> m9658::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
